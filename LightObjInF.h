#ifndef _LIGHT_OBJ_INF_H_
#define _LIGHT_OBJ_INF_H_
#include <DirectXMath.h>
#include <string>

class LightObjInF
{
public:
	LightObjInF() {};
	virtual ~LightObjInF() {};
	DirectX::XMFLOAT4 Color;
	std::string Name;
	float Intensity;
};
class PointLightObj : public LightObjInF
{
public:
	PointLightObj() {};
	virtual ~PointLightObj(){}
	float Radius;
	DirectX::XMFLOAT3 Position;
};
class DirectionLightObj : public LightObjInF
{
public:
	DirectionLightObj() {};
	virtual ~DirectionLightObj() {};
	DirectX::XMFLOAT3 Direction;
	DirectX::XMFLOAT3 Position;
};
class SpotLightObj : public LightObjInF
{
public:
	SpotLightObj() {};
	virtual ~SpotLightObj() {};
	DirectX::XMFLOAT3 Direction;
	float OuterAngle;
	float InnerAngle;
};

#endif // !_LIGHT_OBJ_INF_H_

