#ifndef _POST_FX_INF_H_
#define _POST_FX_INF_H_
#include <Windows.h>
#include "DXInF.h"
struct PostFXParameter
{

};
class PostFXInF
{
public:
	PostFXInF() {};
	virtual ~PostFXInF() {};
	virtual HRESULT Initial(DXInF* pDevice, PostFXParameter* pParameter = nullptr)=0;
	virtual void Compute(DXInF* pDevice, PostFXParameter* pParameter = nullptr) = 0;
	virtual void Destroy() = 0;
};
#endif // !_POST_FX_INF_H_

