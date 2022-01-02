#include "common.fx"
#include "PBR.fx"
#include "InDirectLight.fx"
Texture2D<float> DepthTexture					: register(t0);
Texture2D<float4> ColorSpecIntTexture			: register(t1);
Texture2D<float3> NormalTexture					: register(t2);
Texture2D<float4> SpecPowTexture				: register(t3);
Texture2DArray<float> CascadeShadowMapTexture 	: register(t4);
Texture3D<float4> VoxelTexture					: register(t5);
//transparent texture
Texture3D<float> DepthTexture3D					: register(t6);
Texture3D<float4> ColorSpecIntTexture3D			: register(t7);
Texture3D<float3> NormalTexture3D				: register(t8);
Texture3D<float4> SpecPowTexture3D				: register(t9);

SamplerState PointSampler						: register(s0);
SamplerComparisonState PCFSampler 				: register(s1);		
SamplerState BlockerSampler 					: register(s2);
SamplerState ClampSampler 						: register(s3);

cbuffer cbDirLight : register(b2)
{
	float3 DirToLight			: packoffset(c0);
	float intensity				: packoffset(c0.w);
	float4 DirLightColor		: packoffset(c1);
	matrix ToShadowSpace		: packoffset(c2);
	float4 ToCascadeOffsetX     : packoffset(c6);
	float4 ToCascadeOffsetY		: packoffset(c7);
	float4 ToCascadeScale		: packoffset(c8);
	float ShadowMapPixelSize	: packoffset(c9.x);
	float LightSize				: packoffset(c9.y);
	float isTransparent			: packoffset(c9.z);
}

cbuffer VoxelCB : register(b3)
{
	float3 worldMinPoint : packoffset(c0);
	float3 worldMaxPoint : packoffset(c1);
	float voxelScale : packoffset(c1.w);
	uint volumeDimension : packoffset(c2.x);
	float samplingFactor : packoffset(c2.y);
	float boundStength : packoffset(c2.z);
	float maxTracingDistanceGlobal : packoffset(c2.w);
}

struct VS_OUTPUT
{
	float4 position : SV_POSITION; // vertex position 
	float2 tex : TEXCOORD0;
	float2 cpPos	: TEXCOORD1;
};

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
VS_OUTPUT VSMain(float3 position : POSITION, float2 tex : TEXCOORD)
{
	VS_OUTPUT output;

	float texelSize;
	output.position = float4(position, 1);
	output.tex = tex;
	output.cpPos = output.position.xy;
	return output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shaders
/////////////////////////////////////////////////////////////////////////////

static const float2 poissonDisk[16] = {  
	float2( -0.94201624, -0.39906216 ),  
	float2( 0.94558609, -0.76890725 ),  
	float2( -0.094184101, -0.92938870 ),  
	float2( 0.34495938, 0.29387760 ),  
	float2( -0.91588581, 0.45771432 ),  
	float2( -0.81544232, -0.87912464 ),  
	float2( -0.38277543, 0.27676845 ),  
	float2( 0.97484398, 0.75648379 ),  
	float2( 0.44323325, -0.97511554 ),  
	float2( 0.53742981, -0.47373420 ),  
	float2( -0.26496911, -0.41893023 ),  
	float2( 0.79197514, 0.19090188 ),  
	float2( -0.24188840, 0.99706507 ),  
	float2( -0.81409955, 0.91437590 ),  
	float2( 0.19984126, 0.78641367 ),  
	float2( 0.14383161, -0.14100790 )  
}; 

float CascadedShadow(float3 vertexPoint)
{

	// Transform the world position to shadow space
	float4 posShadowSpace = mul(float4(vertexPoint, 1.0), ToShadowSpace);
	// Transform the shadow space position into each cascade position
	float4 posCascadeSpaceX = (ToCascadeOffsetX + posShadowSpace.xxxx) * ToCascadeScale;
	float4 posCascadeSpaceY = (ToCascadeOffsetY + posShadowSpace.yyyy) * ToCascadeScale;

	// Check which cascade we are in
	float4 inCascadeX = abs(posCascadeSpaceX) <= 1.0;
	float4 inCascadeY = abs(posCascadeSpaceY) <= 1.0;
	float4 inCascade = inCascadeX * inCascadeY;

	// Prepare a mask for the highest quality cascade the position is in
	float4 bestCascadeMask = inCascade;
	bestCascadeMask.yzw = (1.0 - bestCascadeMask.x) * bestCascadeMask.yzw;
	bestCascadeMask.zw = (1.0 - bestCascadeMask.y) * bestCascadeMask.zw;
	bestCascadeMask.w = (1.0 - bestCascadeMask.z) * bestCascadeMask.w;
	float bestCascade = dot(bestCascadeMask, float4(0.0, 1.0, 2.0, 3.0));

	// Pick the position in the selected cascade
	float3 UVD;
	UVD.x = dot(posCascadeSpaceX, bestCascadeMask);
	UVD.y = dot(posCascadeSpaceY, bestCascadeMask);
	UVD.z = posShadowSpace.z;

	// Convert to shadow map UV values
	UVD.xy = 0.5 * UVD.xy + 0.5;
	UVD.y = 1.0 - UVD.y;
	
	//cout blocker for compute soft soft PCF
	float blockerCount = 0;
	float avgBlockerDepth = 0;
	
	for(int i =0;i<2;i++)
	{
		for(int j =0;j<2;j++)
		{
			float4 d4 = CascadeShadowMapTexture.GatherRed(BlockerSampler,float3(UVD.xy, bestCascade),int2(i,j));
			float4 b4 = (UVD.z<=d4)? 0.0 :1.0;
			blockerCount += dot(b4,1.0f);
			avgBlockerDepth += dot(d4,b4);
		}
	}
	if(blockerCount <= 0)
	{
		return 1.f;
	}
	// Penumbra width calculation
	avgBlockerDepth /= blockerCount;
	float fRadio = ((UVD.z - avgBlockerDepth) * LightSize)/avgBlockerDepth;
	fRadio *= fRadio;
	
	float bias = 0.0001;
	
	 // Apply the filter
	float shadow = 0;
	for(int index=0;index<16;index++)
	{
		float2 offset = fRadio * ShadowMapPixelSize * poissonDisk[index];
		
		// Compute the hardware PCF value
		shadow += CascadeShadowMapTexture.SampleCmpLevelZero(PCFSampler, float3(UVD.xy + offset, bestCascade), UVD.z-bias);
		// set the shadow to one (fully lit) for positions with no cascade coverage
		
	}
	shadow /= 16.0f;
	
	shadow = saturate(shadow + 1.0 - any(bestCascadeMask));
	return shadow;
}

float3 DoGammaCorrection(float3 color)
{
	//gamma correction
	float3 gammarValue = 1.0 / 2.2;
	return max(pow(color, gammarValue), 0.0f);
}

float3 CalculateDirectionLight(SURFACE_DATA gbd, Material mat, VS_OUTPUT In)
{
	float3 position = CalcWorldPos(In.cpPos, gbd.LinearDepth);
	// Calculate the directional light
	MaterialPBR matPBR;
	matPBR.normal = mat.normal;
	matPBR.diffuseColor = mat.diffuseColor.xyz;
	matPBR.metallic = mat.metallic;
	matPBR.roughness = mat.roughness;
	matPBR.dirLight = DirToLight;
	matPBR.eyePosition = EyePosition;
	matPBR.intensity = intensity;
	matPBR.dirLightColor = DirLightColor.xyz;

	//global illumination
		//InDirectLightParameter input;
		//input.voxelScale = voxelScale;
		//input.voxelDimension = volumeDimension;
		//input.maxTracingDistanceGlobal = maxTracingDistanceGlobal;
		//input.position = position;
		//input.normal = mat.normal;
		//input.diffuseColor = mat.diffuseColor.xyz;
		//input.samplingFactor = samplingFactor;
		//input.boundStength = boundStength;
		//input.worldMin = worldMinPoint;
		//input.worldMax = worldMaxPoint;
		//input.clampSampler = ClampSampler;
		//input.voxelTexture = VoxelTexture;
		//finalColor = CalInDirectLight(input).xyz;
	float3 color = CalLightPBR(position, matPBR);
	float shadowAtt = clamp(CascadedShadow(position), 0.2f, 1.f);
	color = DoGammaCorrection(color);
	color *= shadowAtt;
	return color;
}


float4 DirectionalLightCommand(VS_OUTPUT In)
{
	float3 finalColor = 0.f;
	float totalAlpha = 0.f;
	//do transparent
	if (isTransparent == 1.f)
	{
		for (int i = 0; i < LAYER_SIZE; i++)
		{
			// Unpack the GBuffer
			SURFACE_DATA gbd = UnpackGBuffer_Loc(In.position.xy,
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
			//do light
			float3 color = CalculateDirectionLight(gbd,mat,In);
			//calculate alpha
			float alpha = (1.f - totalAlpha) * mat.diffuseColor.w;
			finalColor =  lerp(finalColor.xyz,color, alpha);
			totalAlpha += alpha;
		}
		totalAlpha = saturate(totalAlpha);
	}
	else
	{
		// Unpack the GBuffer
		SURFACE_DATA gbd = UnpackGBuffer_Loc(In.position.xy,
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
		// Reconstruct the world position
		float3 position = CalcWorldPos(In.cpPos, gbd.LinearDepth);
		//do light
		finalColor = CalculateDirectionLight(gbd, mat, In);
		totalAlpha = mat.diffuseColor.w;
	}
	// Return the final color
	return float4(finalColor, totalAlpha);
}
float4 PSMain(VS_OUTPUT In) : SV_TARGET
{
	return DirectionalLightCommand(In);
}


