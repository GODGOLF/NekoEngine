#include "common.fx"
#include "PBR.fx"
Texture2D<float> DepthTexture					: register(t0);
Texture2D<float4> ColorSpecIntTexture			: register(t1);
Texture2D<float3> NormalTexture					: register(t2);
Texture2D<float4> SpecPowTexture				: register(t3);
SamplerState PointSampler						: register(s0);

cbuffer cbDirLight : register(b2)
{
	float3 AmbientDown			: packoffset(c0);
	float3 AmbientRange			: packoffset(c1);
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
// Ambient light calculation helper function
float3 CalcAmbient(float3 normal, float3 color)
{
	// Convert from [-1, 1] to [0, 1]
	float up = normal.y * 0.5 + 0.5;

	// Calculate the ambient value
	float3 ambient = AmbientDown + up * AmbientRange;

	// Apply the ambient value to the color
	return ambient * color;
}

float4 PSMain(VS_OUTPUT In) : SV_TARGET
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
	float3 finalColor = 0;
	
	if(gbd.shaderTypeID !=0)
	{
		finalColor = mat.diffuseColor.rgb;
	}
	else
	{
		finalColor = CalcAmbient(mat.normal, mat.diffuseColor.rgb);
	}	
	
	return float4(finalColor,mat.diffuseColor.a);
}


