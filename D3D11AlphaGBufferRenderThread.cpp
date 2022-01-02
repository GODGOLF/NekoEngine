#include"DXInclude.h"
#include "D3D11AlphaGBufferRenderThread.h"
#include "D3D11Class.h"
#include "FunctionHelper.h"
#include "OceanObj.h"
#include "AnimeObj.h"
#include "SkyboxObj.h"
#include "ParticleObj.h"
#include "TerrainObj.h"
#include "D3D11TerrainModel.h"
#include "D3D11OceanModel.h"
#include "D3D11ParticleModel.h"
#include "D3D11Model.h"
#define FRUSTUM_CB_INDEX	4

#define DIFFUSE_LAYER_INDEX		0
#define NORMAL_LAYER_INDEX		1
#define DATA_TX_LAYER_INDEX		2
#define DEPTH_LAYER_INDEX		3
#define DEPTH_NON_TRANPARENT	4

#define RES_CB_INDEX		0


const UINT8 STENCIL_CLEAR_VALUE = 0;
const float CLEAR_RENDER_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
const float CLEAR_RENDER_COLOR_DEPTH[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

struct ResSCB
{
	XMUINT2 res;
	float pad[6];
};

D3D11AlphaGBufferRenderThread::D3D11AlphaGBufferRenderThread()
	:m_colorLayerRT(NULL),
	m_depthLayerRT(NULL),
	m_normalLayerRT(NULL),
	m_specPowerLayerRT(NULL),
	m_colorLayerSRV(NULL),
	m_normalLayerSRV(NULL),
	m_specPowerLayerSRV(NULL),
	m_depthLayerSRV(NULL),
	m_colorLayerUAV(NULL),
	m_normalLayerUAV(NULL),
	m_specPowerLayerUAV(NULL),
	m_depthLayerUAV(NULL),
	m_alphaRenderParameter(),
	m_colorLayerRTV(NULL),
	m_normalLayerRTV(NULL),
	m_specPowerLayerRTV(NULL),
	m_depthLayerRTV(NULL),
	m_indexRecord(0),
	m_resCB(NULL)
{
}
D3D11AlphaGBufferRenderThread::~D3D11AlphaGBufferRenderThread()
{
	Destroy();
}
HRESULT D3D11AlphaGBufferRenderThread::Initial(DXInF* pDevice, Parameter* pParameter)
{
	HRESULT hr = S_OK;
	AlphaGBufferInitialParameter* parameter = (AlphaGBufferInitialParameter*)pParameter;
	D3D11GBufferRenderThread::Initial(pDevice, pParameter);

	ID3D11Device* device = ((D3D11Class*)pDevice)->GetDevice();

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = parameter->width;
	desc.Height = parameter->height;
	desc.Depth = TRANSPARENT_LAYER_COUNT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_DEFAULT;
	hr = device->CreateTexture3D(&desc, NULL, &m_colorLayerRT);
	if (FAILED(hr))
	{
		return hr;
	}
	desc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	hr = device->CreateTexture3D(&desc, NULL, &m_normalLayerRT);
	if (FAILED(hr))
	{
		return hr;
	}
	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	hr = device->CreateTexture3D(&desc, NULL, &m_specPowerLayerRT);
	if (FAILED(hr))
	{
		return hr;
	}
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	hr = device->CreateTexture3D(&desc, NULL, &m_depthLayerRT);
	if (FAILED(hr))
	{
		return hr;
	}
	
	// Create a shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(m_colorLayerRT, &srvDesc, &m_colorLayerSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	srvDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	hr = device->CreateShaderResourceView(m_normalLayerRT, &srvDesc, &m_normalLayerSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	hr = device->CreateShaderResourceView(m_specPowerLayerRT, &srvDesc, &m_specPowerLayerSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	hr = device->CreateShaderResourceView(m_depthLayerRT, &srvDesc, &m_depthLayerSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = desc.Depth;
	hr = device->CreateUnorderedAccessView(m_colorLayerRT, &uavDesc, &m_colorLayerUAV);
	if (FAILED(hr))
	{
		return hr;
	}
	uavDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	hr = device->CreateUnorderedAccessView(m_normalLayerRT, &uavDesc, &m_normalLayerUAV);
	if (FAILED(hr))
	{
		return hr;
	}
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	hr = device->CreateUnorderedAccessView(m_specPowerLayerRT, &uavDesc, &m_specPowerLayerUAV);
	if (FAILED(hr))
	{
		return hr;
	}
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	hr = device->CreateUnorderedAccessView(m_depthLayerRT, &uavDesc, &m_depthLayerUAV);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtsvd;
	ZeroMemory(&rtsvd, sizeof(rtsvd));
	rtsvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtsvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
	rtsvd.Texture3D.MipSlice = 0;
	rtsvd.Texture3D.FirstWSlice = 0;
	rtsvd.Texture3D.WSize = TRANSPARENT_LAYER_COUNT;
	//create renderTarget
	hr = device->CreateRenderTargetView(m_colorLayerRT, &rtsvd, &m_colorLayerRTV);
	if (FAILED(hr))
		return hr;
	rtsvd.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	hr = device->CreateRenderTargetView(m_normalLayerRT, &rtsvd, &m_normalLayerRTV);
	if (FAILED(hr))
		return hr;
	rtsvd.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	hr = device->CreateRenderTargetView(m_specPowerLayerRT, &rtsvd, &m_specPowerLayerRTV);
	if (FAILED(hr))
		return hr;
	rtsvd.Format = DXGI_FORMAT_R32_FLOAT;
	hr = device->CreateRenderTargetView(m_depthLayerRT, &rtsvd, &m_depthLayerRTV);
	if (FAILED(hr))
		return hr;

	// Create gBuffer constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(ResSCB);
	hr = device->CreateBuffer(&cbDesc, NULL, &m_resCB);
	if (FAILED(hr))
	{
		return hr;
	}
	

	return S_OK;
}

void D3D11AlphaGBufferRenderThread::Render(DXInF* pDevice, Parameter* pParameter)
{
	
	AlphaGBufferRenderParameter* pAlphaPara = (AlphaGBufferRenderParameter*)pParameter;
	ID3D11RasterizerState* prevState = NULL;
	m_deviceContext->RSGetState(&prevState);
	m_deviceContext->RSSetState(m_RSCullBack);
	m_deviceContext->RSSetViewports(1, &m_defaultVP);
	m_deviceContext->ClearRenderTargetView(m_colorLayerRTV, CLEAR_RENDER_COLOR);
	m_deviceContext->ClearRenderTargetView(m_normalLayerRTV, CLEAR_RENDER_COLOR);
	m_deviceContext->ClearRenderTargetView(m_specPowerLayerRTV, CLEAR_RENDER_COLOR);
	m_deviceContext->ClearRenderTargetView(m_depthLayerRTV, CLEAR_RENDER_COLOR_DEPTH);
	m_indexRecord = 0;
	for (unsigned int i = 0; i < m_RenderParameter->m_modelDataList->size(); i++)
	{
		ModelInF* pModelInfo = m_RenderParameter->m_modelDataList->operator[](i);
		m_indexRecord = i;
		if (m_RenderParameter->m_modelObjectList->count(pModelInfo->GetModelIndex().c_str()) == 0)
		{
			continue;
		}
		if (!pModelInfo->alphaTranparent)
		{
			continue;
		}
		D3D11GBufferRenderThread::Render(pDevice, pParameter);
		D3DShaderInF* pShader;
		m_shaderManager.GetShader(SHADER_TYPE::TRANSPARENT_CLASSIFICATION_SHADER, &pShader);
		pShader->PreRender(m_deviceContext);
		m_deviceContext->CSSetShaderResources(DIFFUSE_LAYER_INDEX, 1, &m_ColorSpecIntensitySRV);
		m_deviceContext->CSSetShaderResources(NORMAL_LAYER_INDEX, 1, &m_NormalSRV);
		m_deviceContext->CSSetShaderResources(DATA_TX_LAYER_INDEX, 1, &m_SpecPowerSRV);
		m_deviceContext->CSSetShaderResources(DEPTH_LAYER_INDEX, 1, &m_DepthStencilSRV);
		m_deviceContext->CSSetShaderResources(DEPTH_NON_TRANPARENT, 1, &m_alphaRenderParameter.nonTranparentDepthSRV);
		m_deviceContext->CSSetUnorderedAccessViews(DIFFUSE_LAYER_INDEX, 1, &m_colorLayerUAV, NULL);
		m_deviceContext->CSSetUnorderedAccessViews(NORMAL_LAYER_INDEX, 1, &m_normalLayerUAV,NULL);
		m_deviceContext->CSSetUnorderedAccessViews(DATA_TX_LAYER_INDEX, 1, &m_specPowerLayerUAV, NULL);
		m_deviceContext->CSSetUnorderedAccessViews(DEPTH_LAYER_INDEX, 1, &m_depthLayerUAV, NULL);
		//update resolution cb
		UpdateResCB();
		m_deviceContext->CSSetConstantBuffers(RES_CB_INDEX, 1, &m_resCB);

		m_deviceContext->Dispatch((UINT)ceilf((m_defaultVP.Width*m_defaultVP.Height)/1024.f),1,1);
		
		ID3D11ShaderResourceView* nullSRV = NULL;
		m_deviceContext->CSSetShaderResources(DIFFUSE_LAYER_INDEX, 1, &nullSRV);
		m_deviceContext->CSSetShaderResources(NORMAL_LAYER_INDEX, 1, &nullSRV);
		m_deviceContext->CSSetShaderResources(DATA_TX_LAYER_INDEX, 1, &nullSRV);
		m_deviceContext->CSSetShaderResources(DEPTH_LAYER_INDEX, 1, &nullSRV);

		ID3D11UnorderedAccessView* nullUAV = NULL;
		m_deviceContext->CSSetUnorderedAccessViews(DIFFUSE_LAYER_INDEX, 1, &nullUAV, NULL);
		m_deviceContext->CSSetUnorderedAccessViews(NORMAL_LAYER_INDEX, 1, &nullUAV, NULL);
		m_deviceContext->CSSetUnorderedAccessViews(DATA_TX_LAYER_INDEX, 1, &nullUAV, NULL);
		m_deviceContext->CSSetUnorderedAccessViews(DEPTH_LAYER_INDEX, 1, &nullUAV, NULL);
		pShader->PostRender(m_deviceContext);
	}
	m_deviceContext->RSSetState(prevState);
	
	
}

void D3D11AlphaGBufferRenderThread::Update(DXInF* pDevice, Parameter* pParameter)
{
	D3D11GBufferRenderThread::Update(pDevice, pParameter);
}

void D3D11AlphaGBufferRenderThread::Destroy()
{
	D3D11GBufferRenderThread::Destroy();
	SAFE_RELEASE(m_colorLayerRT);
	SAFE_RELEASE(m_depthLayerRT);
	SAFE_RELEASE(m_normalLayerRT);
	SAFE_RELEASE(m_specPowerLayerRT);
	SAFE_RELEASE(m_colorLayerSRV);
	SAFE_RELEASE(m_normalLayerSRV);
	SAFE_RELEASE(m_specPowerLayerSRV);
	SAFE_RELEASE(m_depthLayerSRV);
	SAFE_RELEASE(m_colorLayerUAV);
	SAFE_RELEASE(m_normalLayerUAV);
	SAFE_RELEASE(m_specPowerLayerUAV);
	SAFE_RELEASE(m_depthLayerUAV);
	SAFE_RELEASE(m_resCB);
	SAFE_RELEASE(m_colorLayerRTV);
	SAFE_RELEASE(m_normalLayerRTV);
	SAFE_RELEASE(m_specPowerLayerRTV);
	SAFE_RELEASE(m_depthLayerRTV);
	
}
void D3D11AlphaGBufferRenderThread::RenderObj()
{

	m_culling.ConstructFrustum(m_RenderParameter->pCamera->GetFarValue(),
		m_RenderParameter->pCamera->GetProjection(),
		m_RenderParameter->pCamera->GetView());
	//bind frustum data into Constant buffer
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_deviceContext->Map(m_pFrustumCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CB_FRUSTUM* data = (CB_FRUSTUM*)MappedResource.pData;
	for (int index = 0; index < 6; index++)
	{
		XMStoreFloat4(&data->frustumValues[index], m_culling.GetPlane(index));
	}
	m_deviceContext->Unmap(m_pFrustumCB, 0);

	m_deviceContext->GSSetConstantBuffers(FRUSTUM_CB_INDEX, 1, &m_pFrustumCB);
	

	ModelInF* pModelInfo = m_RenderParameter->m_modelDataList->operator[](m_indexRecord);
	if (m_RenderParameter->m_modelObjectList->count(pModelInfo->GetModelIndex().c_str()) == 0)
	{
		goto END_COMMAND;
	}
	if (!pModelInfo->alphaTranparent)
	{
		goto END_COMMAND;
	}
	//get Current Shader that is used
	D3DShaderInF* shader;

	if (DirectXHelper::instantOfByTypeId<TerrainObj>(pModelInfo))
	{
		m_shaderManager.GetShader(SHADER_TYPE::TERRAIN_SHADER, &shader);
		shader->PreRender(m_deviceContext);
		
		//bind MVP to domain shader
		m_mvp.BindConstantMVP(m_deviceContext, m_RenderParameter->pCamera, MVP_SHADER_INPUT::DOMAIN_SHADER);
		D3D11TerrainModelParameterRender* pRenderParameter = new D3D11TerrainModelParameterRender();
		pRenderParameter->pCamera = m_RenderParameter->pCamera;
		pRenderParameter->pModelInfo = pModelInfo;
		pRenderParameter->pMVP = &m_mvp;
		pRenderParameter->tranparent = false;
		D3DModelInF* pModel = m_RenderParameter->m_modelObjectList->operator[](pModelInfo->GetModelIndex().c_str());
		pModel->Render(m_deviceContext, pRenderParameter);
		delete pRenderParameter;
		pRenderParameter = NULL;
		shader->PostRender(m_deviceContext);
		m_mvp.UnbindConstantMVP(m_deviceContext, MVP_SHADER_INPUT::DOMAIN_SHADER);
	}
	else if (DirectXHelper::instantOfByTypeId<OceanObj>(pModelInfo))
	{

		m_shaderManager.GetShader(SHADER_TYPE::OCEAN_SHADER, &shader);
		shader->PreRender(m_deviceContext);
		
		//bind MVP to domain shader
		m_mvp.BindConstantMVP(m_deviceContext, m_RenderParameter->pCamera, MVP_SHADER_INPUT::DOMAIN_SHADER);
		//input data to model
		D3D11OceanModelParameterRender* pRenderParameter = new D3D11OceanModelParameterRender();
		pRenderParameter->pCamera = m_RenderParameter->pCamera;
		pRenderParameter->pModelInfo = pModelInfo;
		pRenderParameter->pMVP = &m_mvp;
		pRenderParameter->tranparent = true;
		OceanObj* obj = (OceanObj*)pModelInfo;
		pRenderParameter->time = obj->m_time;
		for (int j = 0; j < WAVE_COUNT; j++)
		{
			pRenderParameter->waveInfo[j].direction = obj->m_waveInfo[j].direction;
			pRenderParameter->waveInfo[j].steepness = obj->m_waveInfo[j].steepness;
			pRenderParameter->waveInfo[j].waveLength = obj->m_waveInfo[j].waveLength;
		}

		D3DModelInF* pModel = m_RenderParameter->m_modelObjectList->operator[](pModelInfo->GetModelIndex().c_str());
		pModel->Render(m_deviceContext, pRenderParameter);
		delete pRenderParameter;
		pRenderParameter = NULL;
		shader->PostRender(m_deviceContext);
		m_mvp.UnbindConstantMVP(m_deviceContext, MVP_SHADER_INPUT::DOMAIN_SHADER);
	}
	else if (DirectXHelper::instantOfByTypeId<ParticleObj>(pModelInfo))
	{
		m_shaderManager.GetShader(SHADER_TYPE::PARTICLE_SHADER, &shader);
		shader->PreRender(m_deviceContext);
		//input data to model
		D3D11ParticleModelParameterRender* pRenderParameter = new D3D11ParticleModelParameterRender();
		pRenderParameter->pCamera = m_RenderParameter->pCamera;
		pRenderParameter->pModelInfo = pModelInfo;
		pRenderParameter->pMVP = &m_mvp;
		//pRenderParameter->tranparent = m_isTranparent;
		D3DModelInF* pModel = m_RenderParameter->m_modelObjectList->operator[](pModelInfo->GetModelIndex().c_str());
		pModel->Render(m_deviceContext, pRenderParameter);
		shader->PostRender(m_deviceContext);
	}
	else
	{
		m_shaderManager.GetShader(SHADER_TYPE::MODEL_SHADER, &shader);
		shader->PreRender(m_deviceContext);
		D3DModelInF* pModel = m_RenderParameter->m_modelObjectList->operator[](pModelInfo->GetModelIndex().c_str());
		//bind MVP to geometric shader
		m_mvp.BindConstantMVP(m_deviceContext, m_RenderParameter->pCamera, MVP_SHADER_INPUT::GEO_SHADER);
		D3D11ModelParameterRender* pRenderParameter = new D3D11ModelParameterRender();
		pRenderParameter->pCamera = m_RenderParameter->pCamera;
		pRenderParameter->pModelInfo = pModelInfo;
		pRenderParameter->pMVP = &m_mvp;
		pRenderParameter->drawType = D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
		pModel->Render(m_deviceContext, pRenderParameter);
		delete pRenderParameter;
		pRenderParameter = NULL;
		//unbind mvp constant
		m_mvp.UnbindConstantMVP(m_deviceContext, MVP_SHADER_INPUT::GEO_SHADER);
		shader->PostRender(m_deviceContext);
	}
END_COMMAND:
	ID3D11Buffer* nullCB = NULL;
	m_deviceContext->GSSetConstantBuffers(FRUSTUM_CB_INDEX, 1, &nullCB);
}
void D3D11AlphaGBufferRenderThread::SetAlphaGBufferRenderParameter(AlphaGBufferRenderParameter& parameter)
{
	m_alphaRenderParameter = parameter;
}
void D3D11AlphaGBufferRenderThread::ThreadExcecute()
{
	while (true)
	{
		WaitForSingleObject(m_beginThread, INFINITE);
		Render(NULL, NULL);
		m_deviceContext->FinishCommandList(false, &m_commanList);
		SetEvent(m_endThread);

	}

}
void D3D11AlphaGBufferRenderThread::UpdateResCB()
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_deviceContext->Map(m_resCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	ResSCB* pResValuesCB = (ResSCB*)MappedResource.pData;
	pResValuesCB->res.x = (UINT)m_defaultVP.Width;
	pResValuesCB->res.y = (UINT)m_defaultVP.Height;
	m_deviceContext->Unmap(m_resCB, 0);
}