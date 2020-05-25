#include "MainEditor.h"
#include "EditorCamera.h"
#include "AnimeObj.h"
#include "OceanObj.h"

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
	ObjDesc obj1Dsc;
	obj1Dsc.type = ObjDesc::MODEL_OBJECT;
	obj1Dsc.modelDesc.file = "Data/Models/0113.fbx";
	////test
	m_objScene->AddObj(&m_model.model, obj1Dsc);

	m_model.model->position.z += 5.f;
	m_model.model->scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
	((AnimeObj*)m_model.model)->SetAnimationStackIndex(0);

	////test
	ObjDesc obj2Dsc;
	obj2Dsc.type = ObjDesc::MODEL_OBJECT;
	obj2Dsc.modelDesc.file = "Data/Models/plane.fbx";
	m_objScene->AddObj(&m_model2.model, obj2Dsc);
	m_model2.model->scale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
	CollisionDesc desc;
	desc.rigidModel = RigidModel::PLANE;
	desc.planeDesc.normal = DirectX::XMFLOAT4(0.f, 1.f, 0.f,1.0f);
	m_model2.collision.InitialCollision(m_model2.model,&m_physicManager, desc);

	ObjDesc obj3Dsc;
	obj3Dsc.type = ObjDesc::MODEL_OBJECT;
	obj3Dsc.modelDesc.file = "Data/Models/sphere.fbx";
	m_objScene->AddObj(&m_model3.model, obj3Dsc);
	m_model3.model->scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	m_model3.model->position = DirectX::XMFLOAT3(0.0f, 30.f, 0.0f);
	desc.rigidModel = RigidModel::SPHERE;
	desc.sphereDesc.density = 3.f;
	desc.sphereDesc.radius = 13.f;
	desc.sphereDesc.rigidType = RigidType::DYNAMIC;
	m_model3.collision.InitialCollision(m_model3.model, &m_physicManager, desc);

	ObjDesc obj4Dsc;
	obj4Dsc.type = ObjDesc::TERRAIN_OBJECT;
	obj4Dsc.terrainDesc.file = "Data/Terrain/Height_Map.png";
	obj4Dsc.terrainDesc.diffuseTextureFile = "Data/Terrain/pa.png";
	obj4Dsc.terrainDesc.arraySize = 3;
	obj4Dsc.terrainDesc.ceilSize = 1;
	obj4Dsc.terrainDesc.dimension = 128;
	obj4Dsc.terrainDesc.diffuseColor = DirectX::XMFLOAT4(1, 1, 1, 1);
	m_objScene->AddObj(&m_model4.model , obj4Dsc);
	m_model4.model->scale = DirectX::XMFLOAT3(1.0f, 1.0f,1.0f);
	m_model4.model->position = DirectX::XMFLOAT3(0.0f, 1.f, 0.0f);
	m_model4.model->heightScale = 12.f;
	m_model4.model->minTessFactor = 0;
	m_model4.model->maxTessFactor = 2;
	m_model4.model->maxTessDistance = 50;
	m_model4.model->minTessDistance = 0;
	((PlaneObj*)m_model4.model)->TextureScale = 3.f;

	ObjDesc obj5Dsc;
	obj5Dsc.type = ObjDesc::OCEAN_OBJECT;
	obj5Dsc.oceanDesc.arraySize = 3;
	obj5Dsc.oceanDesc.ceilSize = 1;
	obj5Dsc.oceanDesc.dimension = 128;
	obj5Dsc.oceanDesc.diffuseColor = DirectX::XMFLOAT4(0, 1, 1, 1);
	obj5Dsc.oceanDesc.name = "ocean1";
	obj5Dsc.oceanDesc.normalTexture = "Data/Ocean/waterNormal.jpg";
	m_objScene->AddObj(&m_model5.model, obj5Dsc);
	m_model5.model->scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_model5.model->position = DirectX::XMFLOAT3(0.0f, 6.f, 0.0f);
	m_model5.model->heightScale = 12.f;
	m_model5.model->minTessFactor = 0;
	m_model5.model->maxTessFactor = 2;
	m_model5.model->maxTessDistance = 50;
	m_model5.model->minTessDistance = 0;
	OceanObj* ocean = (OceanObj*)m_model5.model;
	ocean->m_waveInfo[0].direction = DirectX::XMFLOAT2(1, 0);
	ocean->m_waveInfo[0].waveLength = 15.f;
	ocean->m_waveInfo[0].steepness = 0.25f;
	ocean->m_waveInfo[1].direction = DirectX::XMFLOAT2(0, 1);
	ocean->m_waveInfo[1].waveLength = 10.f;
	ocean->m_waveInfo[1].steepness = 0.25f;
	ocean->m_waveInfo[2].direction = DirectX::XMFLOAT2(1, 1);
	ocean->m_waveInfo[2].waveLength = 3.f;
	ocean->m_waveInfo[2].steepness = 0.25f;
	((PlaneObj*)m_model5.model)->TextureScale = 10.f;
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
	DirectX::XMStoreFloat4(&m_model.model->rotation,DirectX::XMQuaternionRotationRollPitchYaw(0.f, radius, 0.f));
	radius += 0.01f;
	
	AnimeObj* anime = ((AnimeObj*)m_model.model);
	anime->SetAnimationTime(time);

	time += 20;
	if (time > anime->GetAnimationStack(anime->GetAnimationStackIndex()).end)
	{
		time = 0;
	}
	OceanObj* c = ((OceanObj*)m_model5.model);
	//update ocean
	c->m_time += 0.01f;
	if (c->m_time > 100.f) 
	{
		c->m_time = 0.f;
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
	
	m_model.Destroy();
	m_model2.Destroy();
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
