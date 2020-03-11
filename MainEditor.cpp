#include "MainEditor.h"

MainEditor::MainEditor() 
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

	//test
	ModelInF* model = NULL;
	char a[] = "Data/Models/SuspCableHolder.fbx";
	m_objScene->AddObj(&a[0], &model);
	return S_OK;
}
void MainEditor::OnUpdate() 
{

	

}
void MainEditor::OnRender(HWND hWnd)
{
	m_engine.OnRender(&m_mainCamera);
}
void MainEditor::OnDestroy() {

	m_engine.OnDestroy();

	if (m_objScene) {
		delete m_objScene;
	}

}
