#include "common.fx"
#include "PBR.fx"
Texture2D<float> DepthTexture					: register(t0);
Texture2D<float4> ColorSpecIntTexture			: register(t1);
Texture2D<float3> NormalTexture					: register(t2);
Texture2D<float4> SpecPowTexture				: register(t3);
SamplerState PointSampler						: register(s0);					

cbuffer cbDirLight : register(b2)
{
	float3 DirToLight			: packoffset(c0);
	float4 DirLightColor		: packoffset(c1);
	float intensity				: packoffset(c2.x);
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


float4 DirectionalLightCommand(VS_OUTPUT In, bool useShadow)
{
	// Unpack the GBuffer
	SURFACE_DATA gbd = UnpackGBuffer_Loc(In.position.xy,
		DepthTexture,
		ColorSpecIntTexture,
		NormalTexture,
		SpecPowTexture,
		PointSampler);

	//// Convert the data into the material structure
	Material mat;
	MaterialFromGBuffer(gbd, mat);

	// Reconstruct the world position
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
	matPBR.dirLightColor = DirLightColor;
	float3 finalColor = CalLightPBR(position, matPBR) ;
	
	//gamma correction
	float3 gammarValue = 1.0 / 2.2;
	finalColor = max(pow(finalColor, gammarValue),0.0f);
	
	// Return the final color
	return float4(finalColor, 1.0f);
}
float4 PSMain(VS_OUTPUT In) : SV_TARGET
{
	return DirectionalLightCommand(In, false);
}


