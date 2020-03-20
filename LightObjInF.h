#ifndef _LIGHT_OBJ_INF_H_
#define _LIGHT_OBJ_INF_H_
#include <DirectXMath.h>
#include <string>

class LightObjInf
{
public:
	LightObjInf() {};
	virtual ~LightObjInf() {};
	DirectX::XMFLOAT3 position;
	std::string name;
};
#endif // !_LIGHT_OBJ_INF_H_

