#include "common.fx"
#include "PBR.fx"

Texture2D<float> DepthTexture			: register(t0);
Texture2D<float4> ColorSpecIntTexture	: register(t1);
Texture2D<float3> NormalTexture			: register(t2);
Texture2D<float4> SpecPowTexture		: register(t3);

Texture3D<float> DepthTexture3D         : register(t4);
Texture3D<float4> ColorSpecIntTexture3D : register(t5);
Texture3D<float3> NormalTexture3D		: register(t6);
Texture3D<float4> SpecPowTexture3D		: register(t7);

SamplerState PointSampler             : register(s0);
/////////////////////////////////////////////////////////////////////////////
// Shadow sampler
/////////////////////////////////////////////////////////////////////////////
SamplerComparisonState PCFSampler : register(s1);

cbuffer cbSpotLightPixel : register(b2)
{
	float3 SpotLightPos					: packoffset(c0);
	float SpotLightRangeRcp 			: packoffset(c0.w);
	float3 SpotDirToLight				: packoffset(c1);
	float SpotCosOuterCone 				: packoffset(c1.w);
	float3 SpotColor					: packoffset(c2);
	float SpotCosConeAttRange 			: packoffset(c2.w);
	float Intensity						: packoffset(c3.x);
	float isTransparent					: packoffset(c3.y);
}

cbuffer cbSpotLightDomain: register(b3)
{
	float4x4 LightProjection : packoffset(c0);
	float SinAngle : packoffset(c4);
	float CosAngle : packoffset(c4.y);
}


/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
float4 VSMain() : SV_Position
{
	return float4(0.0, 0.0, 0.0, 1.0);
}
/////////////////////////////////////////////////////////////////////////////
// Hull shader
/////////////////////////////////////////////////////////////////////////////
struct HS_CONSTANT_DATA_OUTPUT
{
	float Edges[4] : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT PointLightConstantHS()
{
	HS_CONSTANT_DATA_OUTPUT Output;

	float tessFactor = 18.0;
	Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
	Output.Inside[0] = Output.Inside[1] = tessFactor;

	return Output;
}
struct HS_OUTPUT
{
	float4 SpotDir : POSITION;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PointLightConstantHS")]
HS_OUTPUT HSMain(uint PatchID : SV_PrimitiveID)
{
	HS_OUTPUT Output;

	Output.SpotDir = float4(0,0,0,0);

	return Output;
}
/////////////////////////////////////////////////////////////////////////////
// Domain Shader shader
/////////////////////////////////////////////////////////////////////////////
struct DS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 PositionXYW	: TEXCOORD0;
};

#define CylinderPortion 0.2
#define SpherePortion   (1.0 - CylinderPortion)
#define ExpendAmount    (1.0 + CylinderPortion)

[domain("quad")]
DS_OUTPUT DSMain(HS_CONSTANT_DATA_OUTPUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> quad)
{
	// Transform the UV's into clip-space
	float2 posClipSpace = UV.xy * float2(2.0, -2.0) + float2(-1.0, 1.0);

	// Find the vertex offsets based on the UV
	float2 posClipSpaceAbs = abs(posClipSpace.xy);
	float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Force the cone vertices to the mesh edge
	float2 posClipSpaceNoCylAbs = saturate(posClipSpaceAbs * ExpendAmount);
	float maxLenNoCapsule = max(posClipSpaceNoCylAbs.x, posClipSpaceNoCylAbs.y);
	float2 posClipSpaceNoCyl = sign(posClipSpace.xy) * posClipSpaceNoCylAbs;

	// Convert the positions to half sphere with the cone vertices on the edge
	float3 halfSpherePos = normalize(float3(posClipSpaceNoCyl.xy, 1.0 - maxLenNoCapsule));

	// Scale the sphere to the size of the cones rounded base
	halfSpherePos = normalize(float3(halfSpherePos.xy * SinAngle, CosAngle));

	// Find the offsets for the cone vertices (0 for cone base)
	float cylinderOffsetZ = saturate((maxLen * ExpendAmount - 1.0) / CylinderPortion);

	// Offset the cone vertices to thier final position
	float4 posLS = float4(halfSpherePos.xy * (1.0 - cylinderOffsetZ), halfSpherePos.z - cylinderOffsetZ * CosAngle, 1.0);


	// Transform all the way to projected space and generate the UV coordinates
	DS_OUTPUT Output;
	Output.Position = mul(posLS, LightProjection);
	Output.PositionXYW = Output.Position.xyw;

	return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shaders
/////////////////////////////////////////////////////////////////////////////

float3 CalculateSpotLight(in DS_OUTPUT In,in Material mat, in SURFACE_DATA gbd)
{
	float3 finalColor = 0;
	// Reconstruct the world position
	float3 position = CalcWorldPos(In.PositionXYW.xy / In.PositionXYW.z, gbd.LinearDepth);

	float3 ToLight = -(SpotLightPos - position);
	// Calculate the point light by pbr
	MaterialPBR matPBR;
	matPBR.normal = mat.normal;
	matPBR.diffuseColor = mat.diffuseColor.xyz;
	matPBR.metallic = mat.metallic;
	matPBR.roughness = mat.roughness;
	matPBR.dirLight = ToLight;
	matPBR.eyePosition = EyePosition;
	matPBR.intensity = Intensity;
	matPBR.dirLightColor = SpotColor.xyz;
	finalColor = CalLightPBR(position, matPBR);

	// Cone attenuation
	float cosAng = dot(SpotDirToLight, normalize(ToLight));
	float conAtt = saturate((cosAng - SpotCosOuterCone) / SpotCosConeAttRange);
	conAtt *= conAtt;


	// Attenuation
	float DistToLight = length(SpotLightPos - position);
	float DistToLightNorm = 1.0 - saturate(DistToLight * SpotLightRangeRcp);
	float Attn = DistToLightNorm * DistToLightNorm;
	finalColor *= Attn * conAtt;

	return finalColor;
}

float4 spotLightCommand(DS_OUTPUT In)
{
	float4 finalColor =0.f;
	float totalAlpha = 0.f;
	if (isTransparent == 1.f)
	{
		for (int i = 0; i < LAYER_SIZE; i++)
		{
			// Unpack the GBuffer
			SURFACE_DATA gbd = UnpackGBuffer_Loc(In.Position.xy,
				DepthTexture3D,
				ColorSpecIntTexture3D,
				NormalTexture3D,
				SpecPowTexture3D,
				i);
			if (gbd.shaderTypeID != 0)
			{
				continue;
			}
			//// Convert the data into the material structure
			Material mat;
			MaterialFromGBuffer(gbd, mat);
			if (mat.diffuseColor.w <= 0.f)
			{
				continue;
			}
			finalColor.xyz = lerp(finalColor,CalculateSpotLight(In, mat,gbd), mat.diffuseColor.w);
			totalAlpha += mat.diffuseColor.w;
		}
		finalColor.w = saturate(totalAlpha);
	}
	else
	{
		// Unpack the GBuffer
		SURFACE_DATA gbd = UnpackGBuffer_Loc(In.Position.xy,
			DepthTexture,
			ColorSpecIntTexture,
			NormalTexture,
			SpecPowTexture,
			PointSampler);
		if (gbd.shaderTypeID != 0)
		{
			discard;
		}
		//// Convert the data into the material structure
		Material mat;
		MaterialFromGBuffer(gbd, mat);
		finalColor.xyz = CalculateSpotLight(In,mat, gbd);
		totalAlpha = mat.diffuseColor.w;
	}
	finalColor.w = totalAlpha;
	// Return the final color
	return finalColor;
}

float4 PSMain(DS_OUTPUT In) : SV_TARGET
{	
	return spotLightCommand(In);
}



