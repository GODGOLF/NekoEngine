#ifndef _TEXTURE_RECORD_INF_H_
#define _TEXTURE_RECORD_INF_H_
#include "DXInF.h"
class TextureRecordInF
{
public:
	TextureRecordInF() {};
	virtual ~TextureRecordInF() {};
	virtual HRESULT Initial(DXInF* pDevice, int width, int height) =0;
	virtual void BeginRecord(void* pDeviceContext,bool clearDepth, bool clearRenderTarget) =0;
	virtual void EndRecord(void* pDeviceContext) = 0;
	virtual void Destroy() =0;
};

#endif // !_RENDER_TO_TEXTURE_H_
