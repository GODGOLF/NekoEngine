#ifndef _RENDER_H_
#define _RENDER_H_
#include "DXInF.h"
struct Parameter
{

};
class RenderInF
{
public:
	RenderInF() {};
	virtual ~RenderInF() {};

	virtual HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) = 0;

	virtual void Render(DXInF* pDevice,Parameter* pParameter = nullptr) = 0;

	virtual void Update(DXInF* pDevice, Parameter* pParameter = nullptr) = 0;
	
	virtual void Destroy() = 0;
	
};

#endif // !_RENDER_H_

