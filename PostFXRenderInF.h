#ifndef _POST_FX_RENDER_INF_H_
#define _POST_FX_RENDER_INF_H_

#include "DXInF.h"
#include <Windows.h>

struct PostFXRenderParameter
{

};

class PostFXRenderInF
{
public:
	PostFXRenderInF() {}
	~PostFXRenderInF() {}
	virtual HRESULT Initial(DXInF* d3d, PostFXRenderParameter* parameter) = 0;
	virtual void Generate(DXInF* d3d, PostFXRenderParameter* parameter) = 0;
	virtual void Destroy() = 0;
};

#endif // !_POST_FX_RENDER_INF_H_

