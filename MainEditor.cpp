#include "MainEditor.h"
#include "EditorCamera.h"

MainEditor::MainEditor() :m_mainCamera(NULL)
{
	
}

HRESULT MainEditor::OnInit(HWND* hwnd, HINSTANCE hInstance, unsigned int width, unsigned int height) 
{
	
	HRESULT result = m_engine.OnInitial(hwnd, width, height, &m_objScene,&m_light);
	
	if (FAILED(result)) 
	{
		OutputDebugString("Fail to initial Engine\n");
		m_engine.OnDestroy();
		return S_FALSE;
	}
	result = m_engine.CreateImGUIManager(&m_guiEditorManager, hwnd, width, height);
	if (FAILED(result))
	{
		OutputDebugString("Fail to create ImGUI editor\n");
		return S_FALSE;

	}
	bool r = m_inputManager.Initialize(hInstance, hwnd, width, height);
	if (r == false)
	{
		return S_FALSE;
	}
	////test
	char a[] = "Data/Models/011.FBX";
	m_objScene->AddObj(&a[0], &m_model);

	m_model->position.z += 5.f;
	m_model->scale = DirectX::XMFLOAT3(0.2f, 0.2f, .2f);
	m_model->SetAnimationStackIndex(0);

	m_mainCamera = new EditorCamera((int)width, (int)height);
	

	m_DirectionLight.Direction = DirectX::XMFLOAT3(1, 1, 0);
	m_DirectionLight.Color = DirectX::XMFLOAT4(1, 1, 1, 1);
	m_DirectionLight.Intensity = 1.0f;
	m_light->AddLight(&m_DirectionLight);

	//add Gui
	m_guiEditorManager->AddWindow(&m_menuEditor);
	return S_OK;
}
static long long time = 0;
void MainEditor::OnUpdate() 
{
	m_inputManager.Frame();
	bool mouseButton[3];
	mouseButton[0] = m_inputManager.GetMouseLeftClick();
	mouseButton[1] = m_inputManager.GetMouseRightClick();
	mouseButton[2] = m_inputManager.GetMouseRightClick();
	int x,y;
	m_inputManager.GetMouseLocation(x,y);
	m_guiEditorManager->Update(&mouseButton[0], m_inputManager.GetMouseWheel(), DirectX::XMFLOAT2((float)x, (float)y));
	m_model->rotation.y += 0.01f;
	
	m_model->SetAnimationTime(time);

	time += 20;
	if (time > m_model->GetAnimationStack(m_model->GetAnimationStackIndex()).end)
	{
		time = 0;
	}

}
void MainEditor::OnRender(HWND hWnd)
{
	m_engine.OnRender(m_mainCamera);
}
void MainEditor::OnDestroy() {
	
	m_engine.OnDestroy();

	if (m_guiEditorManager)
	{
		delete m_guiEditorManager;
		m_guiEditorManager = NULL;
	}
	if (m_objScene) 
	{
		delete m_objScene;
		m_objScene = NULL;
	}
	if (m_mainCamera)
	{
		delete m_mainCamera;
		m_mainCamera = NULL;
	}
	if (m_light)
	{
		delete m_light;
		m_light = NULL;
	}
	m_inputManager.Shudown();
}
