#include "common.fx"
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

#define FLT_MAX 3.402823e+38


cbuffer MaterialBufferPS : register(b2)
{
	float4 diffuseColor : packoffset(c0);
	float3 haveTexture  : packoffset(c1);
	float shaderTypeID  : packoffset(c1.w);
	float3 camPos 		: packoffset(c2);
	float particleSize	: packoffset(c2.w);
}

cbuffer FrustumCulling : register(b4)
{
	float4 frustumPlanes[6];
}
struct GSInput
{
	float4 position 	: SV_POSITION;
};


struct PSInput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};
GSInput VSMain(float3 position : POSITION)
{
	float4 vInputPos = float4(position, 1.f);
	
	GSInput result;
	result.position = mul(vInputPos, worldMatrix);
	return result;
}



bool FrustumCulling(float3 position, float3 extent)
{
	for(int i=0;i<6;i++)
	{
		float4 plane = frustumPlanes[i];
		float d = dot(extent, abs(plane.xyz));
		float r = dot(position, plane.xyz) + plane.w;
		if((d+r) < 0)
		{
			return false;
		}
	}
	return true;
}
float3 FindCenterTriangle(float3 point0, float3 point1,float3 point2)
{
	//find center
	float3 center;
	center.x = (point0.x + point1.x + point2.x) /3.f;
	center.y = (point0.y + point1.y + point2.y) /3.f;
	center.z = (point0.z + point1.z + point2.z) /3.f;
	return center;
}

[maxvertexcount(4)]
void GSMain(point GSInput input[1], inout TriangleStream<PSInput> triStream)
{
	float3 pos = mul(input[0].position,worldMatrix).xyz ;
	float3 toCamera = camPos - pos;
	toCamera = normalize(toCamera);
	//rotate xz axis
	toCamera.y = 0.0f;
	float3 up = float3(0.0, 1.0f, 0.0);
	float3 right = normalize(cross(toCamera.xyz, up)) * (particleSize/2.f);
	up = float3(0.0, particleSize, 0.0);
	
	float4 tempPos = input[0].position;
	
	//top left 
	PSInput points;
	points.position = mul(tempPos - float4(right, 0.0f) + float4(up, 0.0f), MVP);
	points.tex = float2(0, 0);
	triStream.Append(points);
	//top right
	points.position = mul(tempPos + float4(right, 0.0f) + float4(up, 0.0f) , MVP);
	points.tex = float2(1, 0);
	triStream.Append(points);
	//left down
	points.position = mul(tempPos - float4(right, 0.0f), MVP);
	points.tex = float2(0, 1);
	triStream.Append(points);
	//right down
	points.position = mul(tempPos + float4(right, 0.0f), MVP);
	points.tex = float2(1, 1);
	triStream.Append(points);

	triStream.RestartStrip();
	
}
struct PS_GBUFFER_OUT
{
	float4 ColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};
PS_GBUFFER_OUT PackGBuffer(float4 BaseColor)
{
	PS_GBUFFER_OUT Out;

	// Pack all the data into the GBuffer structure
	Out.ColorSpecInt = BaseColor;
	Out.Normal = float4(1.f,1.f,1.f, 1.f);
	Out.SpecPow = float4(1, 0, 0, shaderTypeID);
	return Out;
}

PS_GBUFFER_OUT PSMain(PSInput input) : SV_TARGET
{
	PS_GBUFFER_OUT output;
	float4 textureColor = diffuseColor;
	float3 newNormal = 0;
	if (haveTexture.x == 1) {
		textureColor = diffuseColor*txDiffuse.Sample(samLinear, input.tex);
	}
	if(textureColor.w == 0.f)
	{
		discard;
	}
	
	output = PackGBuffer(textureColor);
	return output;
}

