#include "MainEditor.h"
#include "EditorCamera.h"

MainEditor::MainEditor() :m_mainCamera(NULL)
{
	
}

HRESULT MainEditor::OnInit(HWND* hwnd, HINSTANCE hInstance, unsigned int width, unsigned int height) 
{
	
	HRESULT result = m_engine.OnInitial(hwnd, width, height, &m_objScene);
	
	if (!SUCCEEDED(result)) 
	{
		OutputDebugString("Error to initial Engine\n");
		m_engine.OnDestroy();
		return S_FALSE;
	}

	////test
	
	char a[] = "Data/Models/SuspCableHolder.fbx";
	m_objScene->AddObj(&a[0], &m_model);

	m_model->position.z += 5.f;

	m_mainCamera = new EditorCamera((int)width, (int)height);
	return S_OK;
}
void MainEditor::OnUpdate() 
{

	m_model->rotation.y += 0.01f;

}
void MainEditor::OnRender(HWND hWnd)
{
	m_engine.OnRender(m_mainCamera);
}
void MainEditor::OnDestroy() {

	m_engine.OnDestroy();

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

}
