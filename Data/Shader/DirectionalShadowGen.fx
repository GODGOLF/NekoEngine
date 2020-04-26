///////////////////////////////////////////////////////////////////
// Spot shadow map generation
///////////////////////////////////////////////////////////////////
#include "common.fx"
cbuffer cbPerMesh : register(b2)
{
	matrix g_mConstBoneWorld[MAX_BONE_MATRICES];
};
cbuffer cbuffercbShadowMapCubeGS : register(b3)
{
	float4x4 CascadeViewProj[3] : packoffset(c0);
};

float4 VSMain(float3 position : POSITION,
	float3 normal : NORMAL,
	float2 tex : TEXCOORD,
	float4 tangent : TANGENT,
	uint4 bone : BONEINDICES,
	float4 weight : WEIGHTS) : SV_Position
{
	float4 vInputPos = float4(position, 1.f);
	float4 vWorldPos = float4(0.f, 0.f, 0.f, 0.f);
	if (bone.x == -1) {
		vWorldPos = vInputPos;
	}
	else {
		matrix boneTransform = g_mConstBoneWorld[bone.x] * weight.x;
		boneTransform += g_mConstBoneWorld[bone.y] * weight.y;
		boneTransform += g_mConstBoneWorld[bone.z] * weight.z;
		boneTransform += g_mConstBoneWorld[bone.w] * weight.w;
		vWorldPos = mul(vInputPos, boneTransform);
	}
	return mul(vWorldPos, worldMatrix);
}
struct GS_OUTPUT
{
	float4 Pos		: SV_POSITION;
	uint RTIndex	: SV_RenderTargetArrayIndex;
};

[maxvertexcount(9)]
void GSMain(triangle float4 InPos[3] : SV_Position, inout TriangleStream<GS_OUTPUT> OutStream)
{
	for (int iFace = 0; iFace < 3; iFace++) {
		GS_OUTPUT output;
		output.RTIndex = iFace;
		for (int v = 0; v < 3; v++)
		{
			output.Pos = mul(InPos[v], CascadeViewProj[iFace]);
			OutStream.Append(output);
		}
		OutStream.RestartStrip();
	}
}