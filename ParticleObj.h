#pragma once
#ifndef _PARTICLE_OBJ_H_
#define _PARTICLE_OBJ_H_
#include "ModelInF.h"

class ParticleObj : public ModelInF
{
public:
	ParticleObj() : speed(1.0f),
		lifeTime(2.0f),
		color(1.f,1.f,1.f,1.f),
		size(1.f),
		direction(0.f,1.f,0.f),
		radius(0.785398f)
	{

	}
	float speed;
	float lifeTime;
	DirectX::XMFLOAT4 color;
	float size;
	DirectX::XMFLOAT3 direction;
	float radius;

};
#endif // !_PARTICLE_OBJ_H_

