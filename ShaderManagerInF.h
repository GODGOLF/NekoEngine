#pragma once
#ifndef _SHADER_MANAGER_INF_
#define _SHADER_MANAGER_INF_
#include "DXInF.h"


class ShaderManagerInF
{
public:
	virtual ~ShaderManagerInF() {};
	virtual HRESULT Initial(DXInF* pDevice) = 0;
	virtual void Destroy() = 0;
};
#endif // !_SHADER_MANAGER_INF_

