#pragma once
#ifndef _IMGUI_RENDER_INF_H_
#define _IMGUI_RENDER_INF_H_
#include "DXInF.h"
#include <Windows.h>
#include "Camera.h"
#include "EditorInF.h"
#include "NekoEngine.h"

class NekoEngine;

class ImGuiRenderInF {
public:
	virtual ~ImGuiRenderInF() {}
	virtual void AddWindow(EditorInf* window) =0;
	virtual void Update(bool* mouseClick,int mouseWheel,DirectX::XMFLOAT2 mousePos)=0;
	virtual void Destroy() =0;
private:
	virtual HRESULT Initial(DXInF* d3d, HWND hwnd, int width, int height) = 0;
	virtual void Render(DXInF* d3d, Camera* camera) = 0;
private:
	friend class NekoEngine;
};

#endif // !_GUI_EDITOR_H_