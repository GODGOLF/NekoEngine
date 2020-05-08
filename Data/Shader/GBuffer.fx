#include "common.fx"
Texture2D txDiffuse : register(t0);
Texture2D ObjNormMap: register(t1);
SamplerState samLinear : register(s0);

#define FLT_MAX 3.402823e+38

cbuffer cbPerMesh : register(b2)
{
	matrix g_mConstBoneWorld[MAX_BONE_MATRICES];
};

cbuffer MaterialBufferPS : register(b3)
{
	float4 diffuseColor;
	float4 specularColor;
	float3 haveTexture;
	float specExp;
	float  metallic;
	float roughness;
}

cbuffer FrustumCulling : register(b4)
{
	float4 frustumPlanes[6];
}
struct GSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};
struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};
GSInput VSMain(float3 position : POSITION,
	float3 normal : NORMAL,
	float2 tex : TEXCOORD,
	float4 tangent : TANGENT,
	float4 binormal : BINORMAL,
	uint4 bone : BONEINDICES,
	float4 weight : WEIGHTS)
{
	float4 vInputPos = float4(position, 1.f);
	float4 vWorldPos = float4(0.f, 0.f, 0.f, 0.f);
	float3 vNormal = normal.xyz;
	
	if (bone.x == -1) {
		vWorldPos = vInputPos;
	}
	else {
		matrix boneTransform = g_mConstBoneWorld[bone.x] * weight.x;
		boneTransform += g_mConstBoneWorld[bone.y] * weight.y;
		boneTransform += g_mConstBoneWorld[bone.z] * weight.z;
		boneTransform += g_mConstBoneWorld[bone.w] * weight.w;
		vWorldPos = mul(vInputPos, boneTransform);
		vNormal = mul(normal, (float3x3)boneTransform);
	}
	GSInput result;
	result.position = mul(vWorldPos, worldMatrix);
	result.normal = mul(vNormal, (float3x3)worldInverse);
	result.normal = normalize(result.normal);
	result.tex = tex;
	//tangent 
	result.tangent = mul(tangent.xyz, (float3x3)worldInverse);
	result.binormal = mul(binormal.xyz,(float3x3)worldInverse);

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
float FindRadianFromTriangle(float3 point0, float3 point1,float3 point2, float3 centerPoint)
{
	//first we have to find min,max value
	float radius =0;
	
	float l = abs(length(point0 - centerPoint));
	if(l>radius)
	{
		radius = l;
	}
	l = abs(length(point1 - centerPoint));
	if(l>radius)
	{
		radius = l;
	}
	l = abs(length(point2 - centerPoint));
	if(l>radius)
	{
		radius = l;
	}
	
	return radius;
	
}
[maxvertexcount(3)]
void GSMain(triangle GSInput input[3], inout TriangleStream<PSInput> triStream)
{
	float3 center = FindCenterTriangle(input[0].position.xyz,input[1].position.xyz,input[2].position.xyz);
	float3 radius = FindRadianFromTriangle(input[0].position.xyz,input[1].position.xyz,input[2].position.xyz,center);
	
	bool isInside = FrustumCulling(center,radius);

	if(!isInside)
	{
		return;
	}
	PSInput output;
	for (int j = 0; j < 3; ++j)
	{
		output.position = mul(input[j].position, viewMatrix);
		output.position = mul(output.position, projectMatrix);
		output.normal =  input[j].normal;
		output.tex = input[j].tex;
		output.tangent = input[j].tangent;
		output.binormal = input[j].binormal;
		triStream.Append(output);
		
	}
	triStream.RestartStrip();
}
struct PS_GBUFFER_OUT
{
	float4 ColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};
PS_GBUFFER_OUT PackGBuffer(float3 BaseColor, float3 Normal, float SpecPower)
{
	PS_GBUFFER_OUT Out;

	// Normalize the specular power
	float SpecPowerNorm = max(0.0001, (SpecPower - g_SpecPowerRange.x) / g_SpecPowerRange.y);

	// Pack all the data into the GBuffer structure
	Out.ColorSpecInt = float4(BaseColor.rgb, 1.0f);
	Out.Normal = float4(Normal * 0.5 + 0.5, 1.0);
	Out.SpecPow = float4(SpecPowerNorm, metallic, roughness, 1.0);
	return Out;
}

PS_GBUFFER_OUT PSMain(PSInput input) : SV_TARGET
{
	PS_GBUFFER_OUT output;
	float4 color;
	float4 textureColor = diffuseColor;
	float3 newNormal = 0;
	if (haveTexture.x != 0) {
		textureColor = diffuseColor*txDiffuse.Sample(samLinear, input.tex);
	}

	//If material has a normal map, we can compute it now
	if (haveTexture.y != 0)
	{
		//Load normal from normal map
		float4 normalMap = ObjNormMap.Sample(samLinear, input.tex);

		//Change normal map range from [0, 1] to [-1, 1]
		normalMap = (2.0f*normalMap) - 1.0f;

		newNormal = (normalMap.x* input.tangent) + (normalMap.y* input.binormal) + (normalMap.z * input.normal);
		
		newNormal = normalize(newNormal);
	
	}
	else
	{
		newNormal= normalize(input.normal);
	}
	output = PackGBuffer(textureColor.xyz, newNormal, specExp);
	return output;
}

