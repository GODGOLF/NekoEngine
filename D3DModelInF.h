

#ifndef _D3DMODEL_INF_H_
#define _D3DMODEL_INF_H_

#include "DXInF.h"
struct ModelExtraParameter
{

};

class D3DModelInF
{
public:
	D3DModelInF() {};
	virtual ~D3DModelInF() { };
	virtual HRESULT Initial(char* file,ModelExtraParameter* parameter = NULL) = 0;
	virtual void Render(void* pDeviceContext, ModelExtraParameter* parameter =NULL)= 0;
	virtual void Destroy() = 0;
};
#endif // !_D3DMODEL_INF_H_
