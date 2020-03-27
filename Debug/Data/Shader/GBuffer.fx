#include "common.fx"
Texture2D txDiffuse : register(t0);
Texture2D ObjNormMap: register(t1);
SamplerState samLinear : register(s0);

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
	float alphaBlend;
	float specIntensity;
}
struct PSInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float4 tangent : TANGENT;
};
PSInput VSMain(float3 position : POSITION,
	float3 normal : NORMAL,
	float2 tex : TEXCOORD,
	float4 tangent : TANGENT,
	uint4 bone : BONEINDICES,
	float4 weight : WEIGHTS)
{
	float4 vInputPos = float4(position, 1.f);
	float4 vWorldPos = float4(0.f, 0.f, 0.f, 0.f);
	float3 vNormal = normal.xyz;
	//if (bone.x == NANI_IDENTIFY) {
		vWorldPos = vInputPos;
	/*}
	else {
		matrix boneTransform = g_mConstBoneWorld[bone.x] * weight.x;
		boneTransform += g_mConstBoneWorld[bone.y] * weight.y;
		boneTransform += g_mConstBoneWorld[bone.z] * weight.z;
		boneTransform += g_mConstBoneWorld[bone.w] * weight.w;
		vWorldPos = mul(vInputPos, boneTransform);
		vNormal = mul(normal, (float3x3)boneTransform);
	}*/
	PSInput result;
	result.position = mul(vWorldPos, MVP);
	result.normal = mul(vNormal, (float3x3)worldInverse);
	result.normal = normalize(result.normal);
	result.tex = tex;
	//tangent 
	result.tangent = mul(tangent, worldInverse);


	return result;
}
struct PS_GBUFFER_OUT
{
	float4 ColorSpecInt : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 SpecPow : SV_TARGET2;
};
PS_GBUFFER_OUT PackGBuffer(float3 BaseColor, float3 Normal, float SpecIntensity, float SpecPower)
{
	PS_GBUFFER_OUT Out;

	// Normalize the specular power
	float SpecPowerNorm = max(0.0001, (SpecPower - g_SpecPowerRange.x) / g_SpecPowerRange.y);

	// Pack all the data into the GBuffer structure
	Out.ColorSpecInt = float4(BaseColor.rgb, SpecIntensity);
	Out.Normal = float4(Normal * 0.5 + 0.5, 1.0);
	Out.SpecPow = float4(SpecPowerNorm, 1.0, 0.0, 1.0);
	return Out;
}

PS_GBUFFER_OUT PSMain(PSInput input) : SV_TARGET
{
	PS_GBUFFER_OUT output;
	float4 color;
	float4 textureColor = diffuseColor;
	if (haveTexture.x != 0) {
		textureColor = diffuseColor*txDiffuse.Sample(samLinear, input.tex);
	}

	//If material has a normal map, we can set it now
	if (haveTexture.y != 0)
	{
		//Load normal from normal map
		float4 normalMap = ObjNormMap.Sample(samLinear, input.tex);

		//Change normal map range from [0, 1] to [-1, 1]
		normalMap = (2.0f*normalMap) - 1.0f;

		//Make sure tangent is completely orthogonal to normal
		float3 newTangent = normalize(input.tangent.xyz - dot(input.tangent.xyz, input.normal)*input.normal);

		//Create the biTangent
		float3 biTangent = cross(input.normal.xyz, newTangent.xyz) * input.tangent.w;

		//Create the "Texture Space"
		float3x3 texSpace = float3x3(newTangent, biTangent.xyz, input.normal.xyz);

		//Convert normal from normal map to texture space and store in input.normal
		input.normal = mul(normalMap.xyz, texSpace);
	}
	output = PackGBuffer(textureColor.xyz, normalize(input.normal), specIntensity, specExp);
	return output;
}

