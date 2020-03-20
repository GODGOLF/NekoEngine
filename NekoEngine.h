#pragma once
#ifndef _NEKO_ENGINE_H_
#define _NEKO_ENGINE_H_
#include "RenderInF.h"
#include <vector>
#include "ObjSceneInF.h"
#include "Camera.h"
class DXInF;

class NekoEngine
{
public:
	NekoEngine();
	HRESULT OnInitial(HWND *hwnd, 
		unsigned int width, 
		unsigned int height,
		ObjScene** pObjScene);
	void OnRender(Camera* pCamera);
	void OnDestroy();
private:
	DXInF* m_pDirectXDevice;
	std::vector<RenderInF*> m_pGBufferRenderThread;
	ObjScene* m_pObjScene;
	RenderInF* m_pWindowRender;

	void PreRender(Camera* pCamera);
	void MainRender(Camera* pCamera);
	void PostProcressRender();

	bool isPreRender;
};

#endif // !_NEKO_ENGINE_H_




