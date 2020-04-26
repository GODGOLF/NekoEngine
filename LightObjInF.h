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

