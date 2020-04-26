#include "DXInclude.h"
#include "D3D11ShadowManagerThread.h"
#include "ObjManager.h"
#include "FunctionHelper.h"
#include "D3D11Class.h"
#include "D3D11Model.h"
#define DIR_SHADOW_CB 3

#define DIRECTIOAL_SHADOW_FILE "Data/Shader/DirectionalShadowGen.fx"

const UINT8 STENCIL_VALUE = 1;

struct CBuffercbShadowMapCubeGS {
	XMMATRIX CascadeViewProj[3];
};

D3D11ShadowManagerThread::D3D11ShadowManagerThread() : m_RSCullBack(NULL),m_DepthStencilState(NULL)
{

}
D3D11ShadowManagerThread::~D3D11ShadowManagerThread()
{
	Destroy();
}

HRESULT D3D11ShadowManagerThread::Initial(DXInF* pDevice, Parameter* pParameter)
{
	ShadowInitialParameter* parameter = (ShadowInitialParameter*)pParameter;
	//transfer data first before send to super data
	RenderThreadInitialParameter superClassParameter;
	superClassParameter.width = parameter->width;
	superClassParameter.height = parameter->height;
	HRESULT hr;
	hr = D3D11RenderThread::Initial(pDevice, &superClassParameter);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_mvp.Initial(pDevice);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11ShaderLayout shaderLayout;
	shaderLayout.layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 64,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	//initial shader
	hr = m_DirectionalLightShadowShader.Initial(pDevice, (char*)DIRECTIOAL_SHADOW_FILE, &shaderLayout, SHADER_MODE::VS_GS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}

	ID3D11Device* device = ((D3D11Class*)pDevice)->GetDevice();

	D3D11_RASTERIZER_DESC rasterDesc;
	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	hr = device->CreateRasterizerState(&rasterDesc, &m_RSCullBack);
	if (FAILED(hr))
		return hr;

	D3D11_DEPTH_STENCIL_DESC descDepth;
	descDepth.DepthEnable = TRUE;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descDepth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	descDepth.StencilEnable = TRUE;
	descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
	descDepth.FrontFace = stencilMarkOp;
	descDepth.BackFace = stencilMarkOp;
	hr = device->CreateDepthStencilState(&descDepth, &m_DepthStencilState);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_RASTERIZER_DESC descRast = {
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK,
		FALSE,
		D3D11_DEFAULT_DEPTH_BIAS,
		D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		FALSE
	};
	descRast.DepthBias = 90;
	descRast.SlopeScaledDepthBias = 2.f;
	descRast.DepthBiasClamp = 0.0f;
	descRast.DepthClipEnable = TRUE;
	hr = device->CreateRasterizerState(&descRast, &m_directionLightRS);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}

void D3D11ShadowManagerThread::Render(DXInF* pDevice, Parameter* pParameter)
{
	//render or light
	for (auto &index : m_dataArray)
	{
		ShadowData* data = index.second;
		if (DirectXHelper::instanceof<DirectionalLightSahdow>(data))
		{
			DirectionalLightSahdow* lightShadow = (DirectionalLightSahdow*)data;
			RenderDirectionalDepthTexture(lightShadow);
		}
	}
}

void D3D11ShadowManagerThread::Update(DXInF* pDevice, Parameter* pParameter)
{
	//reset needDestroy's variable into original becaue we have to check whether light source is still in array or not 
	for (auto &i : m_dataArray)
	{
		i.second->needDestroy = true;
		
	}
	
	auto lightArray = m_shadowParameter->lightmanager->GetLightArray();
	for (unsigned int i = 0; i < lightArray->size(); i++)
	{
		 auto lightData = lightArray->operator[](i);
		 
		 bool isSuccessed = false;
		 if (lightData->DisplayShadow)
		 {
			 decltype(m_dataArray)::iterator it = m_dataArray.find((int)lightData);
			 if (it != m_dataArray.end())
			 {
				 isSuccessed = true;
			 }
			 else
			 {
				 //if the shadow manager still don't allocate shadow data,then create it
				 if (DirectXHelper::instanceof<DirectionLightObj>(lightData))
				 {
					 DirectionalLightSahdow* directionlight = new DirectionalLightSahdow();
					 HRESULT hr = InitialDirectionalLightShadow(directionlight);
					 if (FAILED(hr))
					 {
						 delete directionlight;
					 }
					 else
					 {
						 m_dataArray[(int)lightData] = directionlight;
						 isSuccessed = true;
					 }
					 
				 }
			 }
			 if (isSuccessed)
			 {
				 m_dataArray[(int)lightData]->needDestroy = false;
				 //update light index
				 m_dataArray[(int)lightData]->lightIndex = (int)i;
			 }
			 
		 }
	}
	//remove shadow that it is unnessary
	for (auto i = m_dataArray.begin(); i != m_dataArray.end();)
	{
		if (i->second->needDestroy)
		{
			delete i->second;
			i->second = NULL;
			i = m_dataArray.erase(i);
		}
		else
		{
			i++;
		}
	}
}

void D3D11ShadowManagerThread::Destroy()
{
	D3D11RenderThread::Destroy();

	if (m_shadowParameter)
	{
		delete m_shadowParameter;
	}
	for (auto i = m_dataArray.begin(); i != m_dataArray.end();)
	{
		if (i->second)
		{
			delete i->second;
			i->second = NULL;
		}
		i = m_dataArray.erase(i);
	}
	
	SAFE_RELEASE(m_RSCullBack);
	SAFE_RELEASE(m_DepthStencilState);
	SAFE_RELEASE(m_directionLightRS);
}
void D3D11ShadowManagerThread::ThreadExcecute()
{
	while (true)
	{
		WaitForSingleObject(m_beginThread, INFINITE);
		Update(NULL,NULL);
		Render(NULL, NULL);
		m_deviceContext->FinishCommandList(false, &m_commanList);
		SetEvent(m_endThread);

	}
}
void D3D11ShadowManagerThread::SetShadowDepthRenderParameter(ObjScene* pParameter,LightManager* pLightmanager, Camera* pCamera)
{
	if (m_shadowParameter == NULL)
	{
		m_shadowParameter = new ShadowRenderParameter();
	}
	ObjManager* obj = dynamic_cast<ObjManager*>(pParameter);
	m_shadowParameter->m_modelDataList = obj->GetModelDataList();
	m_shadowParameter->m_modelObjectList = obj->GetModelObjectList();
	m_shadowParameter->lightmanager = pLightmanager;
	m_shadowParameter->pCamera = pCamera;
}
HRESULT D3D11ShadowManagerThread::InitialDirectionalLightShadow(DirectionalLightSahdow* pShadow)
{
	ID3D11Device* pDevice = ((D3D11Class*)m_pDevice)->GetDevice();
	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC dtd;
	dtd.Width = DIRECTIONAL_LIGHT_TEXTURE_SIZE;
	dtd.Height = DIRECTIONAL_LIGHT_TEXTURE_SIZE;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_R32_TYPELESS;
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = 0;
	dtd.ArraySize = 3;
	dtd.MiscFlags = 0;
	ID3D11Texture2D* pCascadedDepthRT = NULL;

	hr = pDevice->CreateTexture2D(&dtd, NULL, &pCascadedDepthRT);
	if (FAILED(hr)) {
		return hr;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDepthView;
	descDepthView.Format = DXGI_FORMAT_D32_FLOAT;
	descDepthView.Flags = 0;
	descDepthView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	descDepthView.Texture2DArray.FirstArraySlice = 0;
	descDepthView.Texture2DArray.ArraySize = CascadedMatrixSet::m_iTotalCascades;
	descDepthView.Texture2DArray.MipSlice = 0;

	hr = pDevice->CreateDepthStencilView(pCascadedDepthRT, &descDepthView, &pShadow->depthStencilDSV);
	if (FAILED(hr)) 
	{
		SAFE_RELEASE(pCascadedDepthRT);
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC descShaderView;
	descShaderView.Format = DXGI_FORMAT_R32_FLOAT;
	descShaderView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	descShaderView.Texture2DArray.FirstArraySlice = 0;
	descShaderView.Texture2DArray.ArraySize = CascadedMatrixSet::m_iTotalCascades;
	descShaderView.Texture2DArray.MipLevels = 1;
	descShaderView.Texture2DArray.MostDetailedMip = 0;

	hr = pDevice->CreateShaderResourceView(pCascadedDepthRT, &descShaderView, &pShadow->depthStencilSRV);
	
	if (FAILED(hr))
	{
		SAFE_RELEASE(pCascadedDepthRT);
		return hr;
	}

	SAFE_RELEASE(pCascadedDepthRT);


	// Create point light constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CBuffercbShadowMapCubeGS);
	hr = pDevice->CreateBuffer(&cbDesc, NULL, &pShadow->pCascadedViewProjCB);
	if (hr != S_OK) {
		return hr;
	}

	pShadow->cascadedMatrix.Init(DIRECTIONAL_LIGHT_TEXTURE_SIZE);

	return hr;
	
}
ShadowData::~ShadowData()
{
	SAFE_RELEASE(depthStencilDSV);
	SAFE_RELEASE(depthStencilSRV);
	
}
DirectionalLightSahdow::~DirectionalLightSahdow()
{
	SAFE_RELEASE(pCascadedViewProjCB);
}

void D3D11ShadowManagerThread::RenderObj()
{

	for (unsigned int i = 0; i < m_shadowParameter->m_modelDataList->size(); i++)
	{
		ModelInF* pModelInfo = m_shadowParameter->m_modelDataList->operator[](i);

		if (m_shadowParameter->m_modelObjectList->count(pModelInfo->GetModelIndex().c_str()) == 0)
		{
			continue;
		}
		D3DModelInF* pModel = m_shadowParameter->m_modelObjectList->operator[](pModelInfo->GetModelIndex().c_str());

		D3D11ModelParameterRender* pRenderParameter = new D3D11ModelParameterRender();
		pRenderParameter->pCamera = m_shadowParameter->pCamera;
		pRenderParameter->pModelInfo = pModelInfo;
		pRenderParameter->pMVP = &m_mvp;
		pModel->Render(m_deviceContext, pRenderParameter);
		delete pRenderParameter;
	}
}
void D3D11ShadowManagerThread::RenderDirectionalDepthTexture(DirectionalLightSahdow* pLightSource)
{
	DirectionLightObj* lightInfo = (DirectionLightObj*)m_shadowParameter->lightmanager->GetLightArray()->operator[](pLightSource->lightIndex);
	pLightSource->cascadedMatrix.Update(lightInfo->Direction,m_shadowParameter->pCamera);
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_deviceContext->Map(pLightSource->pCascadedViewProjCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CBuffercbShadowMapCubeGS* pShadowGenMat = (CBuffercbShadowMapCubeGS*)MappedResource.pData;
	for (int i = 0; i < CascadedMatrixSet::m_iTotalCascades; i++) {
		pShadowGenMat->CascadeViewProj[i] = XMMatrixTranspose(*pLightSource->cascadedMatrix.GetWorldToCascadeProj(i));
	}
	m_deviceContext->Unmap(pLightSource->pCascadedViewProjCB, 0);
	m_deviceContext->GSSetConstantBuffers(DIR_SHADOW_CB, 1, &pLightSource->pCascadedViewProjCB);

	m_DirectionalLightShadowShader.PreRender(m_deviceContext);
	auto lightData = m_shadowParameter->lightmanager->GetLightArray()->operator[](pLightSource->lightIndex);
	pLightSource->cascadedMatrix.Update(((DirectionLightObj*)lightData)->Direction, m_shadowParameter->pCamera);
	
	UINT prevViewportNum =1;
	D3D11_VIEWPORT prevViewport;
	m_deviceContext->RSGetViewports(&prevViewportNum,&prevViewport);
	D3D11_VIEWPORT vp[3] = { { 0, 0, (float)DIRECTIONAL_LIGHT_TEXTURE_SIZE, (float)DIRECTIONAL_LIGHT_TEXTURE_SIZE, 0.0f, 1.0f },
	{ 0, 0, (float)DIRECTIONAL_LIGHT_TEXTURE_SIZE, (float)DIRECTIONAL_LIGHT_TEXTURE_SIZE, 0.0f, 1.0f },
	{ 0, 0, (float)DIRECTIONAL_LIGHT_TEXTURE_SIZE, (float)DIRECTIONAL_LIGHT_TEXTURE_SIZE, 0.0f, 1.0f } };
	m_deviceContext->RSSetViewports(3, vp);

	ID3D11RasterizerState* prevState = NULL;
	m_deviceContext->RSGetState(&prevState);
	m_deviceContext->RSSetState(m_directionLightRS);

	ID3D11DepthStencilState* preDepthState = NULL;
	UINT preStencillValue;
	m_deviceContext->OMGetDepthStencilState(&preDepthState, &preStencillValue);
	m_deviceContext->OMSetDepthStencilState(m_DepthStencilState, STENCIL_VALUE);

	// Set the depth target
	ID3D11RenderTargetView* nullRT = NULL;
	m_deviceContext->OMSetRenderTargets(1, &nullRT, pLightSource->depthStencilDSV);

	// Clear the depth stencil
	m_deviceContext->ClearDepthStencilView(pLightSource->depthStencilDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	RenderObj();

	m_DirectionalLightShadowShader.PostRender(m_deviceContext);
	m_deviceContext->RSSetViewports(prevViewportNum, &prevViewport);
	m_deviceContext->RSSetState(prevState);

	// Set the depth target to NULL
	ID3D11DepthStencilView* nullDepth = NULL;
	m_deviceContext->OMSetRenderTargets(1, &nullRT, nullDepth);
	m_deviceContext->OMSetDepthStencilState(preDepthState, preStencillValue);

	ID3D11Buffer* nullCB = NULL;
	m_deviceContext->GSSetConstantBuffers(DIR_SHADOW_CB, 1, &nullCB);
}
ShadowData* D3D11ShadowManagerThread::GetShadowData(int id)
{
	auto data = m_dataArray.find(id);

	if (data != m_dataArray.end())
	{
		return m_dataArray[id];
	}

	return NULL;
}