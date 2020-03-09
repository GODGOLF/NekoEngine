#include "DXInclude.h"
#include "NekoEngine.h"
#include "D3D11Class.h"
#include "D3D11RenderThread.h"
#include "ObjManager.h"
NekoEngine::NekoEngine()
{
	p_directXDevice = 0;
	
}
HRESULT NekoEngine::OnInitial(HWND *hwnd, 
	unsigned int width, 
	unsigned int height,
	ObjScene** pObjScene)
{
	HRESULT isSucessful;
	p_directXDevice = new D3D11Class();
	isSucessful = p_directXDevice->OnInit(*hwnd, width, height);
	if (!SUCCEEDED(isSucessful))
	{
		return isSucessful;
	}
	//create new obj management 
	*pObjScene = new ObjManager();
	isSucessful = ((ObjManager*)*pObjScene)->Initial(p_directXDevice);
	if (!SUCCEEDED(isSucessful))
	{
		pObjScene = NULL;
		return isSucessful;
	}
	//save pointer in this class
	p_objScene = *pObjScene;
	return S_OK;
}
void NekoEngine::OnDestroy()
{
	if (p_directXDevice) 
	{
		delete p_directXDevice;
	}
	for (unsigned int i = 0; i < p_renderThread.size(); i++)
	{
		delete p_renderThread[i];
	}
}