#include "PBR.fx"
Texture3D<float4> VoxelAlbedo : register(t0);
Texture3D<float3> VoxelNormal : register(t1);
Texture3D<float4> VoxelEmission : register(t2);
RWTexture3D<float4> voxelRadiance : register(u0);

#define PI 3.14159265359

#define EPSILON 1e-30

static const float2 gSpecPowerRange = { 1.0, 250.0 };

#define VOXEL_DIRECTIONAL_LIGHT_MAX	50
struct LightDiretionLight
{
	float3 DirToLight;
	float intensity;
	float4 DirLightColor;
};

struct MaterialSpecular
{
	float specPow;
	float specIntensity;
	float  metallic;
	float roughness;
};

cbuffer cbDirLight : register(b2)
{
	LightDiretionLight directionLight[VOXEL_DIRECTIONAL_LIGHT_MAX];
}

cbuffer VoxelCB : register(b3)
{
	float3 eyePosition : packoffset(c0);
	float3 worldMinPoint : packoffset(c1);;
	float voxelSize : packoffset(c2.x);;
	float voxelScale : packoffset(c2.y);;
	uint volumeDimension : packoffset(c2.z);;
}

float3 VoxelToWorld(int3 pos)
{
	float3 result = float3(pos);
	result *= voxelSize;

	return result + worldMinPoint;
}

float3 WorldToVoxel(float3 position)
{
	float3 voxelPos = position - worldMinPoint;
	return voxelPos * voxelScale;
}
float3 DecodeNormal(float3 normal)
{
	return normal * 2.0f - float3(1.0f, 1.0f, 1.0f);
}
void MaterialFromGBuffer(int4 UVW,inout MaterialSpecular mat)
{
	float4 data = VoxelEmission.Load(UVW);
	mat.specPow = gSpecPowerRange.x + gSpecPowerRange.y * data.x;
	mat.specIntensity = VoxelAlbedo.Load(UVW).w;
	mat.metallic = data.y;
	mat.roughness = data.z;
}



[numthreads(8, 8, 8)]
void CSMain(uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	if (dispatchThreadId.x >= volumeDimension ||
		dispatchThreadId.y >= volumeDimension ||
		dispatchThreadId.z >= volumeDimension) return;
	uint3 writePos = dispatchThreadId;

	
	float4 albedo = VoxelAlbedo.Load(int4(writePos, 0));

	if (albedo.a < EPSILON) 
	{
		voxelRadiance[writePos] = float4(0.f,0.f,0.f,0.f);
		return; 
	}
	albedo.a = 0.f;
	float3 normal = DecodeNormal(VoxelNormal.Load(int4(writePos, 0)));
	
	//load material
	MaterialSpecular mat;
	MaterialFromGBuffer(int4(writePos, 0),mat);

	if (albedo.r > 0.f || albedo.g > 0.f || albedo.b >0.f ) 
	{
		float3 wsPosition = VoxelToWorld(writePos);
		for(int i =0;i<VOXEL_DIRECTIONAL_LIGHT_MAX;i++)
		{
			LightDiretionLight light = directionLight[i];
			if(light.intensity == -1.f)
			{
				break;
			}
			// Calculate the directional light
			MaterialPBR matPBR;
			matPBR.normal = normal;
			matPBR.diffuseColor = albedo.xyz;
			matPBR.metallic = mat.metallic;
			matPBR.roughness = mat.roughness;
			matPBR.dirLight = light.DirToLight;
			matPBR.eyePosition = eyePosition;
			matPBR.intensity = light.intensity;
			matPBR.dirLightColor = light.DirLightColor.xyz;
			albedo.xyz = CalLightPBR(wsPosition, matPBR) ;
		}
	}
	albedo.a = 1.f;
	voxelRadiance[writePos] = albedo;
}