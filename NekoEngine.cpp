#include "DXInclude.h"
#include "NekoEngine.h"
#include "D3D11Class.h"
#include "D3D11RenderThread.h"
#include "ObjManager.h"
#include "D3D11GBufferRenderThread.h"
#include "D3D11AlphaGBufferRenderThread.h"
#include "D3D11WindowRender.h"
#include "D3D11LightRenderManager.h"
#include "LightManager.h"
#include "D3D11ImGuiRender.h"
#include "D3D11VoxelizationThread.h"
#include "D3D11ShadowManagerThread.h"
#include "D3D11TextureCombine.h"
#include <dxgidebug.h>
NekoEngine::NekoEngine()
{
	m_pDirectXDevice = 0;
	isPreRender = true;
	m_pWindowRender = NULL;
	m_imguiRender = NULL;
	m_pLightAlphaRender = NULL;
	m_pLightRender = NULL;
	m_textureCombine = NULL;
	
}
HRESULT NekoEngine::OnInitial(HWND *hwnd, 
	unsigned int width, 
	unsigned int height,
	ObjScene** pObjScene,
	LightManagementInF** pLightManager)
{
	HRESULT isSucessful;
	m_pDirectXDevice = new D3D11Class();
	isSucessful = m_pDirectXDevice->OnInit(*hwnd, width, height);
	if (!SUCCEEDED(isSucessful))
	{
		return isSucessful;
	}
	//create new obj management 
	*pObjScene = new ObjManager();
	isSucessful = ((ObjManager*)*pObjScene)->Initial(m_pDirectXDevice);
	if (!SUCCEEDED(isSucessful))
	{
		delete *pObjScene;
		pObjScene = NULL;
		return isSucessful;
	}
	//save pointer in this class
	m_pObjScene = *pObjScene;

	//create gBuffer's object
	m_renderThread.push_back(new D3D11GBufferRenderThread());
	GBufferInitialParameter gBufferParameter;
	gBufferParameter.width = width;
	gBufferParameter.height = height;
	m_renderThread.back()->Initial(m_pDirectXDevice, &gBufferParameter);

	//create voxel thread
	m_renderThread.push_back(new D3D11VoxelizationThread());
	m_renderThread.back()->Initial(m_pDirectXDevice);

	//light
	m_pLightRender = new D3D11LightRenderManager();
	LightInitialParameter lightParameter;
	lightParameter.width = width;
	lightParameter.height = height;
	m_pLightRender->Initial(m_pDirectXDevice, &lightParameter);

	m_pLightAlphaRender = new D3D11LightRenderManager();
	m_pLightAlphaRender->Initial(m_pDirectXDevice, &lightParameter);

	//create light manager that store light's info 
	*pLightManager = new LightManager();
	m_lightObj = *pLightManager;

	//final render
	m_pWindowRender = new D3D11WindowRender();
	m_pWindowRender->Initial(m_pDirectXDevice);

	//Shadow Manager
	m_renderThread.push_back(new D3D11ShadowManagerThread());
	ShadowInitialParameter shadowParameter;
	shadowParameter.width = width;
	shadowParameter.height = height;
	m_renderThread.back()->Initial(m_pDirectXDevice, &shadowParameter);

	//alpha Gbuffer
	m_secondRenderThread.push_back(new D3D11AlphaGBufferRenderThread());
	m_secondRenderThread.back()->Initial(m_pDirectXDevice, &gBufferParameter);

	//texture combination
	m_textureCombine = new D3D11TextureCombine();
	m_textureCombine->Initial(m_pDirectXDevice, width, height);


	return S_OK;
}
void NekoEngine::OnDestroy()
{
	if (m_pDirectXDevice)
	{
		delete m_pDirectXDevice;
	}
	for (auto& i : m_renderThread)
	{
		delete i;
		i = NULL;
	}
	for (auto& i : m_secondRenderThread)
	{
		delete i;
		i = NULL;
	}
	if (m_pWindowRender)
	{
		delete m_pWindowRender;

	}
	if (m_pLightRender)
	{
		delete m_pLightRender;
	}
	if (m_pLightAlphaRender)
	{
		delete m_pLightAlphaRender;
	}
	m_imguiRender = NULL;
	if (m_textureCombine)
	{
		delete m_textureCombine;
	}
	
}
void NekoEngine::OnRender(Camera* pCamera)
{
	if (isPreRender)
	{
		PreRender(pCamera);
		isPreRender = false;
	}
	D3D11Class* device = (D3D11Class*)m_pDirectXDevice;
	/////////////////////////////////////////////////////////////////////////////////
	//draw multi-thread for first time 
	//create empty array to store Handle
	std::vector<HANDLE> handleArray;
	handleArray.resize(m_renderThread.size());
	int i = 0;
	for (auto &threadObj : m_renderThread)
	{
		D3D11RenderThread* pGBufferRender = (D3D11RenderThread*)threadObj;
		//signal ready for scene kickoff
		pGBufferRender->BindBeginEventHandle();
		handleArray[i] = pGBufferRender->GetEndThreadHandle();
		i++;
	}
	// wait for completion
	WaitForMultipleObjects(handleArray.size(),
		&handleArray[0],
		TRUE,
		INFINITE);
	//clear handle array;
	ZeroMemory(&handleArray, sizeof(handleArray));

	//Execute all CommandList from the thread
	for (auto &threadObj : m_renderThread)
	{
		D3D11RenderThread* pGBufferRender = (D3D11RenderThread*)threadObj;
		pGBufferRender->ExecuteAndReleaseCommandList(device);
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//draw for mult-thread for second time
	handleArray.clear();
	handleArray.resize(m_secondRenderThread.size());
	i = 0;
	for (auto& threadObj : m_secondRenderThread)
	{
		D3D11RenderThread* pGBufferRender = (D3D11RenderThread*)threadObj;
		//signal ready for scene kickoff
		pGBufferRender->BindBeginEventHandle();
		handleArray[i] = pGBufferRender->GetEndThreadHandle();
		i++;
	}
	// wait for completion
	WaitForMultipleObjects(handleArray.size(),
		&handleArray[0],
		TRUE,
		INFINITE);
	//clear handle array;
	ZeroMemory(&handleArray, sizeof(handleArray));

	//Execute all CommandList from the thread
	for (auto& threadObj : m_secondRenderThread)
	{
		D3D11RenderThread* pGBufferRender = (D3D11RenderThread*)threadObj;
		pGBufferRender->ExecuteAndReleaseCommandList(device);
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	//draw Scene in the main Thread(lighting)
	MainRender(pCamera);

	//draw Post Process Render
	PostProcressRender();

	//draw final of result of Scene 	
	device->BindMainRenderTarget(); 
	//hard code
	WindowRenderParameter rParamter;
	rParamter.texture = (ID3D11ShaderResourceView*)m_textureCombine->GetOutput();
	m_pWindowRender->Render(m_pDirectXDevice, &rParamter);

	//draw imGui
	if (m_imguiRender)
	{
		m_imguiRender->Render(m_pDirectXDevice, pCamera);
	}
	//last thing is calling this function
	device->EndDraw();
}
void NekoEngine::PreRender(Camera* pCamera)
{
	//set necessary variable for gBuffer into the thread
	((D3D11GBufferRenderThread*)m_renderThread[0])->SetGBufferRenderParameter(m_pObjScene,pCamera);
	((D3D11VoxelizationThread*)m_renderThread[1])->SetGBufferRenderParameter((LightManager*)m_lightObj, m_pObjScene, pCamera);
	((D3D11ShadowManagerThread*)m_renderThread[2])->SetShadowDepthRenderParameter(m_pObjScene, (LightManager*)m_lightObj, pCamera);
	AlphaGBufferRenderParameter alphaRenderParameter;
	alphaRenderParameter.nonTranparentDepthSRV = ((D3D11GBufferRenderThread*)m_renderThread[0])->GetDepthView();
	((D3D11AlphaGBufferRenderThread*)m_secondRenderThread[0])->SetGBufferRenderParameter(m_pObjScene, pCamera);
	((D3D11AlphaGBufferRenderThread*)m_secondRenderThread[0])->SetAlphaGBufferRenderParameter(alphaRenderParameter);
}
void NekoEngine::MainRender(Camera* pCamera)
{
	LightRenderParameter lightParameter;
	lightParameter.pCamera = pCamera;
	//get data from gBuffer
	D3D11GBufferRenderThread* gBuffer = (D3D11GBufferRenderThread*)m_renderThread[0];
	lightParameter.colorSRV = gBuffer->GetColorView();
	lightParameter.depthStencilDSV = gBuffer->GetDepthView();
	lightParameter.normalSRV = gBuffer->GetNormalView();
	lightParameter.specPowerSRV = gBuffer->GetSpecPowerView();
	lightParameter.pLights = ((LightManager*)m_lightObj)->GetLightArray();
	lightParameter.shadowManager = (D3D11ShadowManagerThread*)m_renderThread[2];
	lightParameter.voxelLightPassSRV = ((D3D11VoxelizationThread*)m_renderThread[1])->GetVoxelLightPassSRV();
	lightParameter.voxelLightRenderCB = ((D3D11VoxelizationThread*)m_renderThread[1])->GetVoxelLightRenderCB();
	lightParameter.transparent = false;
	//draw light!
	m_pLightRender->Render(m_pDirectXDevice, &lightParameter);
	
	////do tranparent
	ZeroMemory(&lightParameter, sizeof(lightParameter));
	lightParameter.pCamera = pCamera;
	D3D11GBufferRenderThread* alphaGBuffer = (D3D11GBufferRenderThread*)m_secondRenderThread[0];
	lightParameter.colorSRV = alphaGBuffer->GetColorView();
	lightParameter.depthStencilDSV = alphaGBuffer->GetDepthView();
	lightParameter.normalSRV = alphaGBuffer->GetNormalView();
	lightParameter.specPowerSRV = alphaGBuffer->GetSpecPowerView();
	lightParameter.pLights = ((LightManager*)m_lightObj)->GetLightArray();
	lightParameter.shadowManager = (D3D11ShadowManagerThread*)m_renderThread[2];
	lightParameter.voxelLightPassSRV = ((D3D11VoxelizationThread*)m_renderThread[1])->GetVoxelLightPassSRV();
	lightParameter.voxelLightRenderCB = ((D3D11VoxelizationThread*)m_renderThread[1])->GetVoxelLightRenderCB();
	lightParameter.transparent = true;
	//draw light!
	m_pLightAlphaRender->Render(m_pDirectXDevice, &lightParameter);
	//
	D3D11Class* pDevice = (D3D11Class*)m_pDirectXDevice;
	//combine transparent texture and non transparent text
	m_textureCombine->Render(pDevice->GetDeviceContext(),((D3D11LightRenderManager*)m_pLightRender)->GetLightSRV(),
		((D3D11LightRenderManager*)m_pLightAlphaRender)->GetLightSRV(),
		gBuffer->GetDepthView(), alphaGBuffer->GetDepthView());
	
}
void NekoEngine::PostProcressRender()
{

}
HRESULT NekoEngine::CreateImGUIManager(ImGuiRenderInF** pManager, HWND *hwnd,
	unsigned int width,
	unsigned int height)
{
	HRESULT hr;
	*pManager = new D3D11ImGUIRender();
	hr = (*pManager)->Initial(m_pDirectXDevice, *hwnd, width, height);
	if (FAILED(hr))
	{
		(*pManager)->Destroy();
		delete (*pManager);
		(*pManager) = NULL;
		return hr;
	}
	m_imguiRender = *pManager;
	return S_OK;
}