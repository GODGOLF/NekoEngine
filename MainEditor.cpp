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
	r = m_physicManager.Initial();
	if (r == false)
	{
		return S_FALSE;
	}
	////test
	char a[] = "Data/Models/0113.fbx";
	m_objScene->AddObj(&a[0], &m_model);

	m_model.position.z += 5.f;
	m_model.scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
	m_model.SetAnimationStackIndex(0);

	////test
	char b[] = "Data/Models/plane.fbx";
	m_objScene->AddObj(&b[0], &m_model2);
	m_model2.scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
	CollisionDesc desc;
	desc.rigidModel = RigidModel::PLANE;
	desc.planeDesc.normal = DirectX::XMFLOAT4(0.f, 1.f, 0.f,1.0f);
	m_model2.InitialCollision(&m_physicManager, desc);
	m_model2.SetAnimationStackIndex(0);

	char c[] = "Data/Models/sphere.fbx";
	m_objScene->AddObj(&c[0], &m_model3);
	m_model3.scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	m_model3.position = DirectX::XMFLOAT3(0.0f, 30.f, 0.0f);
	m_model3.SetAnimationStackIndex(0);
	desc.rigidModel = RigidModel::SPHERE;
	desc.sphereDesc.density = 3.f;
	desc.sphereDesc.radius = 13.f;
	desc.sphereDesc.rigidType = RigidType::DYNAMIC;
	m_model3.InitialCollision(&m_physicManager, desc);

	//create main camera
	m_mainCamera = new EditorCamera((int)width, (int)height);
	

	m_DirectionLight.Direction = DirectX::XMFLOAT3(-1, -1, 0);
	m_DirectionLight.Color = DirectX::XMFLOAT4(1, 1, 1, 1);
	m_DirectionLight.Intensity = 3.0f;
	m_DirectionLight.DisplayShadow = true;
	m_light->AddLight(&m_DirectionLight);

	m_pointLight.Color = DirectX::XMFLOAT4(1, 0, 0,1);
	m_pointLight.Intensity = 3.f;
	m_pointLight.Position = DirectX::XMFLOAT3(1, 3, 1);
	m_pointLight.Radius = 15.f;
	m_light->AddLight(&m_pointLight);

	m_spotLight.Color = DirectX::XMFLOAT4(0, 1, 0, 1);
	m_spotLight.Intensity = 3.f;
	m_spotLight.Position = DirectX::XMFLOAT3(4, 6, 1);
	m_spotLight.Radius = 15.f;
	m_spotLight.InnerAngle = 30.f;
	m_spotLight.OuterAngle = 40.f;
	m_spotLight.Direction = DirectX::XMFLOAT3(0, -1, 0);
	m_light->AddLight(&m_spotLight);

	//add Gui
	m_guiEditorManager->AddWindow(&m_menuEditor);
	return S_OK;
}
static long long time = 0;
static float radius = 0.f;
void MainEditor::OnUpdate() 
{
	//update input
	m_inputManager.Frame();
	bool mouseButton[3];
	mouseButton[0] = m_inputManager.GetMouseLeftClick();
	mouseButton[1] = m_inputManager.GetMouseRightClick();
	mouseButton[2] = m_inputManager.GetMouseRightClick();
	int x,y;
	m_inputManager.GetMouseLocation(x,y);
	m_guiEditorManager->Update(&mouseButton[0], m_inputManager.GetMouseWheel(), DirectX::XMFLOAT2((float)x, (float)y));
	
	//update model
	DirectX::XMStoreFloat4(&m_model.rotation,DirectX::XMQuaternionRotationRollPitchYaw(0.f, radius, 0.f));
	radius += 0.01f;
	
	m_model.SetAnimationTime(time);

	time += 20;
	if (time > m_model.GetAnimationStack(m_model.GetAnimationStackIndex()).end)
	{
		time = 0;
	}
	//physic
	const physx::PxF32 timeStep = 1.f / 60.f;
	m_physicManager.Update(timeStep);
	m_model3.Update();
	((EditorCamera*)m_mainCamera)->Update(&m_inputManager);

}
void MainEditor::OnRender(HWND hWnd)
{
	m_engine.OnRender(m_mainCamera);
}
void MainEditor::OnDestroy() {
	

	m_model3.Destroy();
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

	m_physicManager.Destroy();
}
