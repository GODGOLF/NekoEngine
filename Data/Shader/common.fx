#define MAX_BONE_MATRICES 200
#define PI 3.14159265359
#define HALF_PI 3.14159265359/2.f
static const float2 g_SpecPowerRange = { 1.0, 250.0 };
cbuffer cbPerObjectVS : register(b0)
{
	matrix MVP;
	matrix projectMatrix;
	matrix worldMatrix;
	matrix viewMatrix;
	matrix worldInverse;
}

cbuffer cbGBufferUnpack : register(b1)
{
	float4 PerspectiveValues : packoffset(c0);
	float4x4 ViewInv         : packoffset(c1);
}
#define EyePosition	(ViewInv[3].xyz)
struct SURFACE_DATA
{
	float LinearDepth;
	float4 Color;
	float3 Normal;
	float SpecPow;
	float SpecIntensity;
	float  metallic;
	float roughness;
	float shaderTypeID;
};
float ConvertZToLinearDepth(float depth)
{
	float linearDepth = PerspectiveValues.z / (depth + PerspectiveValues.w);
	return linearDepth;
}
SURFACE_DATA UnpackGBuffer(float2 UV, 
	Texture2D<float> DepthTexture,
	Texture2D<float4> ColorSpecIntTexture,
	Texture2D<float3> NormalTexture,
	Texture2D<float4> SpecPowTexture,
	SamplerState PointSampler)
{
	SURFACE_DATA Out;

	float depth = DepthTexture.Sample(PointSampler, UV.xy).x;
	Out.LinearDepth = ConvertZToLinearDepth(depth);
	float4 baseColorSpecInt = ColorSpecIntTexture.Sample(PointSampler, UV.xy);
	Out.Color = baseColorSpecInt;
	Out.SpecIntensity = baseColorSpecInt.w;
	Out.Normal = NormalTexture.Sample(PointSampler, UV.xy).xyz;
	Out.Normal = normalize(Out.Normal * 2.0 - 1.0);
	Out.SpecPow = SpecPowTexture.Sample(PointSampler, UV.xy).x;
	Out.metallic = SpecPowTexture.Sample(PointSampler, UV.xy).y;
	Out.roughness = SpecPowTexture.Sample(PointSampler, UV.xy).z;
	return Out;
}
SURFACE_DATA UnpackGBuffer_Loc(int2 location, 
	Texture2D<float> DepthTexture,
	Texture2D<float4> ColorSpecIntTexture,
	Texture2D<float3> NormalTexture,
	Texture2D<float4> SpecPowTexture,
	SamplerState PointSampler)
{
	SURFACE_DATA Out;
	int3 location3 = int3(location, 0);

	float depth = DepthTexture.Load(location3).x;
	Out.LinearDepth = ConvertZToLinearDepth(depth);
	float4 baseColorSpecInt = ColorSpecIntTexture.Load(location3);
	Out.Color = baseColorSpecInt;
	Out.SpecIntensity = baseColorSpecInt.w;
	Out.Normal = NormalTexture.Load(location3).xyz;
	Out.Normal = normalize(Out.Normal * 2.0 - 1.0);
	Out.SpecPow = SpecPowTexture.Load(location3).x;
	Out.metallic = SpecPowTexture.Load(location3).y;
	Out.roughness = SpecPowTexture.Load(location3).z;
	Out.shaderTypeID = SpecPowTexture.Load(location3).w;
	return Out;
}
struct Material
{
	float3 normal;
	float4 diffuseColor;
	float specPow;
	float specIntensity;
	float  metallic;
	float roughness;
};

void MaterialFromGBuffer(SURFACE_DATA gbd, inout Material mat)
{
	mat.normal = gbd.Normal;
	mat.diffuseColor = gbd.Color;
	mat.specPow = g_SpecPowerRange.x + g_SpecPowerRange.y * gbd.SpecPow;
	mat.specIntensity = gbd.SpecIntensity;
	mat.metallic = gbd.metallic;
	mat.roughness = gbd.roughness;
}
float3 CalcWorldPos(float2 csPos, float depth)
{
	float4 position;

	position.xy = csPos.xy * PerspectiveValues.xy * depth;
	position.z = depth;
	position.w = 1.0;

	return mul(position, ViewInv).xyz;
}

float3 world_to_svo(float3 posW, float voxel_size, float3 offset)
{
	float3 pos = posW;
	pos = ((pos + offset) / voxel_size);
	return pos;
}

float3 svo_to_world(float3 posW, float voxel_size, float3 offset)
{
	float3 pos = posW;
	pos *= voxel_size;
	pos -= offset;
	return pos;
}
