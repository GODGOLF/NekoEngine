#pragma once

#ifndef _D3D_SHADER_INF_H_
#define _D3D_SHADER_INF_H_
#include <Windows.h>
#include "DXInF.h"
#include <vector>

struct SHADER_MODE
{
	enum VALUE {
		VS_MODE,
		PS_MODE,
		VS_PS_MODE,
		VS_PS_HS_DS_MODE,
		VS_PS_GS_MODE,
		VS_GS_MODE,
		CS_MODE
	};
};
struct ShaderLayout
{

};

class D3DShaderInF
{
public:
	virtual void PreRender(DXInF* pDevice) =0;
	virtual void PostRender(DXInF* pDevice) =0;
	virtual void PreRender(void* pDeviceContext) = 0;
	virtual void PostRender(void* pDeviceContext) = 0;

	virtual	 HRESULT Initial(DXInF* pDevice, char* file, ShaderLayout* layout, SHADER_MODE::VALUE mode) =0;
	virtual void Destroy() = 0;
	
};
#endif // !_SCENE_DEFFER_SHADER_H_
