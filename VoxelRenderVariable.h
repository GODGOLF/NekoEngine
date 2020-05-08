#ifndef _VOXEL_RENDER_VARIABLE_H_
#define _VOXEL_RENDER_VARIABLE_H_
#include "GlobalVariable.h"
struct VoxelRenderVariable : GlobalVariable
{
	VoxelRenderVariable() :samplingFactor(0.5f),boundStength(1.f), maxTracingDistanceGlobal(1.0f)
	{

	}
	float samplingFactor;
	float boundStength;
	float maxTracingDistanceGlobal;
};

#endif // !_VOXEL_RENDER_VARIABLE_H_
