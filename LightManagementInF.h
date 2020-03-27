#ifndef _LIGHT_MANAGEMENT_INF_H_
#define _LIGHT_MANAGEMENT_INF_H_ 
#include <Windows.h>
#include "LightObjInF.h"
struct LIGHT_TYPE
{
	enum VALUE
	{
		DIRECTION_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT
	};
};
class LightManagementInF
{
public:
	LightManagementInF() {};
	virtual ~LightManagementInF() {};
	/*
		Direction light's function support only one light;
	*/
	virtual bool AddLight(LightObjInF* pLightObj) = 0;
	virtual bool RemoveLight(LightObjInF* pLightObj) = 0;
	virtual void Destroy() = 0;
};

#endif // !_OBJ_SCENE_INF_H_

