#include "DXInclude.h"
#include "NekoEngine.h"
#include "D3D11Class.h"
#include "D3D11RenderThread.h"
#include "ObjManager.h"
#include "D3D11GBufferRenderThread.h"
#include "D3D11WindowRender.h"
#include "D3D11LightRenderManager.h"
#include "LightManager.h"
#include "D3D11ImGuiRender.h"
NekoEngine::NekoEngine()
{
	m_pDirectXDevice = 0;
	isPreRender = true;
	m_pWindowRender = NULL;
	m_imguiRender = NULL;
	
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
		pObjScene = NULL;
		return isSucessful;
	}
	//save pointer in this class
	m_pObjScene = *pObjScene;

	//create gBuffer's object
	m_pGBufferRenderThread.push_back(new D3D11GBufferRenderThread());
	GBufferInitialParameter gBufferParameter;
	gBufferParameter.width = width;
	gBufferParameter.height = height;
	m_pGBufferRenderThread.back()->Initial(m_pDirectXDevice, &gBufferParameter);

	//light
	m_pLightRender = new D3D11LightRenderManager();
	LightInitialParameter lightParameter;
	lightParameter.width = width;
	lightParameter.height = height;
	m_pLightRender->Initial(m_pDirectXDevice, &lightParameter);

	//create light manager that store light's info 
	*pLightManager = new LightManager();
	m_lightObj = *pLightManager;

	//final render
	m_pWindowRender = new D3D11WindowRender();
	m_pWindowRender->Initial(m_pDirectXDevice);

	return S_OK;
}
void NekoEngine::OnDestroy()
{
	if (m_pDirectXDevice) 
	{
		delete m_pDirectXDevice;
	}
	for (auto &i : m_pGBufferRenderThread)
	{
		delete i;
	}
	if (m_pWindowRender)
	{
		delete m_pWindowRender;

	}
	if (m_pLightRender)
	{
		delete m_pLightRender;
	}
	m_imguiRender = NULL;
	
}
void NekoEngine::OnRender(Camera* pCamera)
{
	if (isPreRender)
	{
		PreRender(pCamera);
		isPreRender = false;
	}
	//get array size
	const int numHandle = 1;
	int* pNum = const_cast<int*>(&numHandle);
	*pNum = (int)m_pGBufferRenderThread.size();
	
	HANDLE handleArray[numHandle];
	int i = 0;
	for (auto &threadObj : m_pGBufferRenderThread)
	{
		D3D11RenderThread* pGBufferRender = (D3D11RenderThread*)threadObj;
		//signal ready for scene kickoff
		pGBufferRender->BindBeginEventHandle();
		handleArray[i] = pGBufferRender->GetEndThreadHandle();
		i++;
	}
	// wait for completion
	WaitForMultipleObjects(numHandle,
		handleArray,
		TRUE,
		INFINITE);

	D3D11Class* device = (D3D11Class*)m_pDirectXDevice;
	//Execute all CommandList from the thread
	for (auto &threadObj : m_pGBufferRenderThread)
	{
		D3D11RenderThread* pGBufferRender = (D3D11RenderThread*)threadObj;
		ID3D11CommandList* pCommanList = pGBufferRender->GetCommandList();
		device->GetDeviceContext()->ExecuteCommandList(pCommanList, false);
		SAFE_RELEASE(pCommanList);
	}
	
	//draw Scene in the main Thread(lighting)
	MainRender(pCamera);

	//draw Post Process Render
	PostProcressRender();

	//draw final of result of Scene 	
	device->BindMainRenderTarget(); 
	//hard code
	WindowRenderParameter rParamter;
	rParamter.texture = ((D3D11LightRenderManager*)m_pLightRender)->GetLightSRV();
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
	((D3D11GBufferRenderThread*)m_pGBufferRenderThread[0])->SetGBufferRenderParameter(m_pObjScene,pCamera);
	
}
void NekoEngine::MainRender(Camera* pCamera)
{
	LightRenderParameter lightParameter;
	lightParameter.pCamera = pCamera;
	//get data from gBuffer
	D3D11GBufferRenderThread* gBuffer = (D3D11GBufferRenderThread*)m_pGBufferRenderThread[0];
	lightParameter.colorSRV = gBuffer->GetColorView();
	lightParameter.depthStencilDSV = gBuffer->GetDepthView();
	lightParameter.normalSRV = gBuffer->GetNormalView();
	lightParameter.specPowerSRV = gBuffer->GetSpecPowerView();
	lightParameter.pLights = ((LightManager*)m_lightObj)->GetLightArray();
	//draw light!
	m_pLightRender->Render(m_pDirectXDevice, &lightParameter);
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