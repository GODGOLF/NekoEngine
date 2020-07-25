#pragma once
#ifndef _NEKO_ENGINE_H_
#define _NEKO_ENGINE_H_
#include "RenderInF.h"
#include <vector>
#include "ObjSceneInF.h"
#include "Camera.h"
#include "LightManagementInF.h"
#include "ImGuiIRenderInF.h"
#include "TextureCombineInF.h"
class DXInF;

class ImGuiRenderInF;

class NekoEngine
{
public:
	NekoEngine();
	HRESULT OnInitial(HWND *hwnd, 
		unsigned int width, 
		unsigned int height,
		ObjScene** pObjScene,
		LightManagementInF** pLightManager);
	void OnRender(Camera* pCamera);
	HRESULT CreateImGUIManager(ImGuiRenderInF** pManager,HWND *hwnd,
		unsigned int width,
		unsigned int height);
	void OnDestroy();
private:
	DXInF* m_pDirectXDevice;
	std::vector<RenderInF*> m_renderThread;
	std::vector<RenderInF*> m_secondRenderThread;
	ObjScene* m_pObjScene;
	//render result of drawing
	RenderInF* m_pWindowRender;
	
	RenderInF* m_pLightRender;
	RenderInF* m_pLightAlphaRender;
	LightManagementInF* m_lightObj;
	bool isPreRender;

	ImGuiRenderInF* m_imguiRender;

	TextureCombineInF* m_textureCombine;
private:
	void PreRender(Camera* pCamera);
	void MainRender(Camera* pCamera);
	void PostProcressRender();

};

#endif // !_NEKO_ENGINE_H_




