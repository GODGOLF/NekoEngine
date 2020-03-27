#pragma once
#ifndef _LIGHT_RENDER_INTERFACE_H_
#define _LIGHT_RENDER_INTERFACE_H_
#include "Camera.h"
#include <Windows.h>
#include "DXInF.h"
#include "LightObjInF.h"
struct LightParameter
{
	
};

class LightRenderInF 
{
public:
	LightRenderInF() { };
	virtual ~LightRenderInF() {};
	virtual HRESULT Initial(DXInF* pDevice) =0;
	virtual void Render(void* pDeviceContext, 
		LightObjInF* pObj,
		Camera* pCamera,
		LightParameter* extraParameter =NULL) =0;
	virtual void Destroy() = 0;
};

#endif // !_LIGHT_INTERFACE_H_
