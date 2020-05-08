Texture3D<float4> VoxelTexture : register(t0);
RWTexture3D<float4> voxelRadiance[6] : register(u0);

groupshared float SharedPositions[8];


#define PI 3.14159265359

#define EPSILON 1e-30

static const float2 gSpecPowerRange = { 1.0, 250.0 };

static const int3 anisoOffsets[] = 
(
	int3(1, 1, 1),
	int3(1, 1, 0),
	int3(1, 0, 1),
	int3(1, 0, 0),
	int3(0, 1, 1),
	int3(0, 1, 0),
	int3(0, 0, 1),
	int3(0, 0, 0)
);


cbuffer VoxelCB : register(b3)
{
	uint volumeDimension : packoffset(c0.x);
}


void FetchTexels(uint3 pos, inout float4 val[8]) 
{
	for(int i=0;i<8;i++)
	{
		val[i] = VoxelTexture[pos].rgb;	
	}
}


[numthreads(8, 8, 8)]
void CSMain(uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	if (dispatchThreadId.x >= volumeDimension ||
		dispatchThreadId.y >= volumeDimension ||
		dispatchThreadId.z >= volumeDimension) return;
	uint3 writePos = dispatchThreadId;
	ivec3 sourcePos = writePos * 2;
	float4 values[8];
	x-
	FetchTexels(sourcePos, values);
	SharedPositions[groupThreadId] = float4((
		values[0] + values[4] * (1 - values[0].a) + 
		values[1] + values[5] * (1 - values[1].a) +
		values[2] + values[6] * (1 - values[2].a) +
		values[3] + values[7] * (1 - values[3].a) * 0.25f), 1.0f);
	x+
	SharedPositions[groupThreadId] = float4((
		values[0] + values[4] * (1 - values[0].a) + 
		values[1] + values[5] * (1 - values[1].a) +
		values[2] + values[6] * (1 - values[2].a) +
		values[3] + values[7] * (1 - values[3].a) * 0.25f), 1.0f);
	
	
}