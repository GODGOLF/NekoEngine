#include "common.fx"
#include "PBR.fx"

Texture2D<float> DepthTexture         : register(t0);
Texture2D<float4> ColorSpecIntTexture : register(t1);
Texture2D<float3> NormalTexture       : register(t2);
Texture2D<float4> SpecPowTexture      : register(t3);
SamplerState PointSampler             : register(s0);
/////////////////////////////////////////////////////////////////////////////
// Shadow sampler
/////////////////////////////////////////////////////////////////////////////
SamplerComparisonState PCFSampler : register(s1);

cbuffer cbPointLightPixel : register(b2)
{
	float3 PointLightPos : packoffset(c0);
	float PointLightRangeRcp : packoffset(c0.w);
	float3 PointColor : packoffset(c1);
	float2 LightPerspectiveValues	: packoffset(c2);
	float Intensity : packoffset(c2.z);
}
cbuffer cbPointLightDomain : register(b3)
{
	float4x4 LightProjection : packoffset(c0);
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
	float3 HemiDir : POSITION;
};

static const float3 HemilDir[2] = {
	float3(1.0, 1.0,1.0),
	float3(-1.0, 1.0, -1.0)
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PointLightConstantHS")]
HS_OUTPUT HSMain(uint PatchID : SV_PrimitiveID)
{
	HS_OUTPUT Output;

	Output.HemiDir = HemilDir[PatchID];

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

[domain("quad")]
DS_OUTPUT DSMain(HS_CONSTANT_DATA_OUTPUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> quad)
{
	// Transform the UV's into clip-space
	float2 posClipSpace = UV.xy * 2.0 - 1.0;

	// Find the absulate maximum distance from the center
	float2 posClipSpaceAbs = abs(posClipSpace.xy);
	float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Generate the final position in clip-space
	float3 normDir = normalize(float3(posClipSpace.xy, (maxLen - 1)) * quad[0].HemiDir);
	float4 posLS = float4(normDir, 1.0);

	// Transform all the way to projected space
	DS_OUTPUT Output;
	Output.Position = mul(posLS, LightProjection);

	// Store the clip space position
	Output.PositionXYW = Output.Position.xyw;

	return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shaders
/////////////////////////////////////////////////////////////////////////////
float4 pointLightCommand(DS_OUTPUT In)
{
	// Unpack the GBuffer
	SURFACE_DATA gbd = UnpackGBuffer_Loc(In.Position.xy,
		DepthTexture,
		ColorSpecIntTexture,
		NormalTexture,
		SpecPowTexture,
		PointSampler);

	//// Convert the data into the material structure
	Material mat;
	MaterialFromGBuffer(gbd, mat);
	// Reconstruct the world position
	float3 position = CalcWorldPos(In.PositionXYW.xy / In.PositionXYW.z, gbd.LinearDepth);
	
	float3 ToLight = -(PointLightPos - position);
	// Calculate the point light by pbr
	MaterialPBR matPBR;
	matPBR.normal = mat.normal;
	matPBR.diffuseColor = mat.diffuseColor.xyz;
	matPBR.metallic = mat.metallic;
	matPBR.roughness = mat.roughness;
	matPBR.dirLight = ToLight;
	matPBR.eyePosition = EyePosition;
	matPBR.intensity = Intensity;
	matPBR.dirLightColor = PointColor.xyz;
	float3 finalColor = CalLightPBR(position, matPBR) ;
	
	// Attenuation
	float DistToLight = length(ToLight);
	float DistToLightNorm = 1.0 - saturate(DistToLight * PointLightRangeRcp);
	float Attn = DistToLightNorm * DistToLightNorm;
	
	return float4(finalColor* Attn, 1.0f);
}

float4 PSMain(DS_OUTPUT In) : SV_TARGET
{
	return pointLightCommand(In);
}


