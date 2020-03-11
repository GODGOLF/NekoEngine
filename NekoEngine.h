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
	void OnRender(Camera* camera);
	void OnDestroy();
private:
	DXInF* p_directXDevice;
	std::vector<RenderInF*> p_renderThread;
	ObjScene* p_objScene;
};

#endif // !_NEKO_ENGINE_H_




