#include "common.fx"
#include "PBR.fx"

Texture2D<float> DepthTexture					: register(t0);
Texture2D<float4> ColorSpecIntTexture			: register(t1);
Texture2D<float3> NormalTexture					: register(t2);
Texture2D<float4> SpecPowTexture				: register(t3);

Texture3D<float> DepthTexture3D					: register(t4);
Texture3D<float4> ColorSpecIntTexture3D			: register(t5);
Texture3D<float3> NormalTexture3D				: register(t6);
Texture3D<float4> SpecPowTexture3D				: register(t7);

SamplerState PointSampler						: register(s0);

cbuffer cbDirLight : register(b2)
{
	float3 AmbientDown			: packoffset(c0);
	float3 AmbientRange			: packoffset(c1);
	float isTransparent			: packoffset(c2.x);
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
	float3 finalColor = 0.f;
	float totalAlpha = 0.f;
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
			//// Convert the data into the material structure
			Material mat;
			MaterialFromGBuffer(gbd, mat);
			if (mat.diffuseColor.w <=0.f)
			{
				break;
			}
			
			float alpha = (1.f - totalAlpha) * mat.diffuseColor.w;
			if (gbd.shaderTypeID != 0)
			{
				finalColor = lerp(finalColor, mat.diffuseColor.rgb, alpha);
			}
			else
			{
				finalColor = lerp(finalColor, CalcAmbient(mat.normal, mat.diffuseColor.rgb), alpha);
				//finalColor = prevColor.xyz * prevColor.w + (1 - prevColor.w) * CalcAmbient(mat.normal, mat.diffuseColor.rgb);
			}
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
		//// Convert the data into the material structure
		Material mat;
		MaterialFromGBuffer(gbd, mat);

		if (gbd.shaderTypeID != 0)
		{
			finalColor = mat.diffuseColor.rgb;
		}
		else
		{
			finalColor = CalcAmbient(mat.normal, mat.diffuseColor.rgb);
		}
		totalAlpha = mat.diffuseColor.a;
	}
	return float4(finalColor, totalAlpha);
}


