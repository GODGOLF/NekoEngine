#ifndef _MAIN_EDITOR_
#define _MAIN_EDITOR_
#include<Windows.h>
#include <wrl.h>
#include <shellapi.h>
#include "ScreenInF.h"
#include "NekoInF.h"
#include "ObjSceneInF.h"
#include "MenuBarEditor.h"
#include "KMInputManager.h"
#include "PhysicManager.h"
#include "ModelProperty.h"
#include "ModelHierachyUI.h"
#include "ImGuiIRenderInF.h"

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
	NekoInF* m_engine;
	ObjScene* m_objScene;
	LightManagementInF* m_light;
	DirectionLightObj m_DirectionLight;
	PointLightObj m_pointLight;
	SpotLightObj m_spotLight;

	std::vector<ModelProperty> m_models;
	Camera* m_mainCamera;
	ModelProperty m_model;
	ModelProperty m_model2;
	ModelProperty m_model3;
	ModelProperty m_model4; 
	ModelProperty m_model5;
	ModelProperty m_model6;
	ModelProperty m_particle;
	ImGuiRenderInF* m_guiEditorManager;
	MenuBarEditor m_menuEditor;
	ModelHierachyUI m_ModelHierachyEditor;

	KMInputManager m_inputManager;

	PhysicManager m_physicManager;
};
#endif // !D3D_SYSTEM

