#ifndef _MAIN_EDITOR_
#define _MAIN_EDITOR_
#include<Windows.h>
#include <wrl.h>
#include <shellapi.h>
#include "ScreenInF.h"
#include "NekoEngine.h"
#include "ObjSceneInF.h"
#include "MenuBarEditor.h"
#include "KMInputManager.h"

class MainEditor : public ScreenInF
{
public:
	MainEditor();
	virtual HRESULT OnInit(HWND *hwnd, HINSTANCE hInstance, unsigned int width, unsigned int height) override;
	virtual void OnUpdate()  override;
	virtual void OnRender(HWND hWnd) override;
	virtual void OnDestroy() override;
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
	
private:
	NekoEngine m_engine;
	ObjScene* m_objScene;
	LightManagementInF* m_light;
	DirectionLightObj m_DirectionLight;

	Camera* m_mainCamera;
	ModelInF* m_model = NULL;
	ModelInF* m_model2 = NULL;

	ImGuiRenderInF* m_guiEditorManager;
	MenuBarEditor m_menuEditor;

	KMInputManager m_inputManager;
};
#endif // !D3D_SYSTEM

