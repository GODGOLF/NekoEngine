#include "NekoInF.h"
#include "NekoEngine.h"

static bool g_initial = false;

NekoInF* NekoInF::CreateEngineManager(HWND* hwnd,
	unsigned int width,
	unsigned int height,
	ObjScene** pObjScene,
	LightManagementInF** pLightManager)
{
	if (g_initial)
	{
		return NULL;
	}
	NekoInF*output = new NekoEngine();
	HRESULT hr = ((NekoEngine*)output)->OnInitial(hwnd, width, height, pObjScene, pLightManager);
	if (FAILED(hr))
	{
		delete output;
		return NULL;
	}
	return output;
}