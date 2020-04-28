#ifndef _LIGHT_OBJ_INF_H_
#define _LIGHT_OBJ_INF_H_
#include <DirectXMath.h>
#include <string>

class LightObjInF
{
public:
	LightObjInF() : Intensity(1),Name(""),Color(1.f,1.f,1.f,1.f),DisplayShadow(false) {} ;
	virtual ~LightObjInF() {};
	DirectX::XMFLOAT4 Color;
	std::string Name;
	float Intensity;
	bool DisplayShadow;
};
class PointLightObj : public LightObjInF
{
public:
	PointLightObj() :Radius(1), Position(0,0,0)
	{};
	virtual ~PointLightObj(){}
	float Radius;
	DirectX::XMFLOAT3 Position;
};
class DirectionLightObj : public LightObjInF
{
public:
	DirectionLightObj() : Direction(1,0,0){};
	virtual ~DirectionLightObj() {};
	DirectX::XMFLOAT3 Direction;
};
class SpotLightObj : public LightObjInF
{
public:
	SpotLightObj() : Direction(1,1,0),OuterAngle(3.12f),InnerAngle(3.12f/2.f),Position(0,0,0), Radius(1) {};
	virtual ~SpotLightObj() {};
	DirectX::XMFLOAT3 Direction;
	float OuterAngle;
	float InnerAngle;
	DirectX::XMFLOAT3 Position;
	float Radius;
};

#endif // !_LIGHT_OBJ_INF_H_

