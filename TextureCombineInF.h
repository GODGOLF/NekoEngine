#ifndef _TEXTURE_COMBINE_INTERFACE_H_
#define _TEXTURE_COMBINE_INTERFACE_H_

#include "DXInF.h"
#include <Windows.h>
class TextureCombineInF
{
public:
	virtual ~TextureCombineInF()
	{

	}
	virtual HRESULT Initial(DXInF* pD3Device,int width, int height) = 0;
	virtual void Render(void* pDeviceContext, 
		void* textureInput1, 
		void*textureInput2, 
		void* depthInput1,
		void* depthInput2) = 0;
	virtual void Destroy() = 0;
	virtual void* GetOutput() = 0;
};
#endif // !_TEXTURE_COMBINE_INTERFACE_H_
