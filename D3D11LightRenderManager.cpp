#include "DXInclude.h"
#include "D3D11LightRenderManager.h"
#include "D3D11DirectionalLightRender.h"
#include "D3D11Class.h"
#include "D3D11TextureRecord.h"
#include "FunctionHelper.h"
#include "D3D11AmbientLightRender.h"
#include "D3D11PointLightRender.h"
#include "D3D11SpotLightRender.h"
#define PREPARE_FOR_UNPACK_INDEX	1
#define VOXEL_LIGHT_RENDER_CB_INDEX 3
#define VOXEL_LIGHT_SRV_INDEX		5
#define VOXEL_CLAMP_SAMPLER			3

struct CB_GBUFFER_UNPACK
{
	XMFLOAT4 PerspectiveValues;
	XMMATRIX  ViewInv;
};

D3D11LightRenderManager::D3D11LightRenderManager() 
	: m_pDirectionLight(NULL),
	m_pAddBlendState(NULL),
	m_pAdditiveBlendState(NULL),
	m_pNoDepthClipFrontRS(NULL),
	m_pNoDepthWriteGreatherStencilMaskState(NULL),
	m_pGBufferUnpackCB(NULL)
{

}
D3D11LightRenderManager::~D3D11LightRenderManager()
{
	Destroy();
}

HRESULT D3D11LightRenderManager::Initial(DXInF* pDevice, Parameter* pParameter)
{
	HRESULT hr;
	m_pDirectionLight = new D3D11DirectionalLightRender();
	hr = m_pDirectionLight->Initial(pDevice);
	if (FAILED(hr))
	{
		return hr;
	}
	m_pAmbientLight = new D3D11AmbientLightRender();
	hr = m_pAmbientLight->Initial(pDevice);
	if (FAILED(hr))
	{
		return hr;
	}
	m_pPointLight = new D3D11PointLightRender();
	hr = m_pPointLight->Initial(pDevice);
	if (FAILED(hr))
	{
		return hr;
	}
	m_pSpotLight = new D3D11SpotLightRender();
	hr = m_pSpotLight->Initial(pDevice);
	if (FAILED(hr))
	{
		return hr;
	}
	LightInitialParameter* pLightParameter = (LightInitialParameter*)pParameter;
	ID3D11Device* pD3D11Device = ((D3D11Class*)pDevice)->GetDevice();
	D3D11_DEPTH_STENCIL_DESC descDepth;
	descDepth.DepthEnable = TRUE;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descDepth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	descDepth.StencilEnable = TRUE;
	descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	/*const D3D11_DEPTH_STENCILOP_DESC noSkyStencilOp = {
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
		D3D11_COMPARISON_EQUAL };*/
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
	descDepth.FrontFace = stencilMarkOp;
	descDepth.BackFace = stencilMarkOp;
	// Stencil operations if pixel is front-facing.
	hr = pD3D11Device->CreateDepthStencilState(&descDepth, &m_pDepthWriteLessStencilMaskState);
	if (FAILED(hr))
	{
		return hr;
	}
	descDepth.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	const D3D11_DEPTH_STENCILOP_DESC stencilRepleaseMarkOp = { D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS };
	hr = pD3D11Device->CreateDepthStencilState(&descDepth, &m_pNoDepthWriteGreatherStencilMaskState);
	if (FAILED(hr))
	{
		return hr;
	}
	// Create the additive blend state
	D3D11_BLEND_DESC descBlend;
	descBlend.AlphaToCoverageEnable = FALSE;
	descBlend.IndependentBlendEnable = FALSE;
	const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		TRUE,
		D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		descBlend.RenderTarget[i] = defaultRenderTargetBlendDesc;

	hr = pD3D11Device->CreateBlendState(&descBlend, &m_pAdditiveBlendState);
	if (FAILED(hr))
	{
		return hr;
	}
	const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc2 =
	{
		TRUE,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		descBlend.RenderTarget[i] = defaultRenderTargetBlendDesc2;

	hr = pD3D11Device->CreateBlendState(&descBlend, &m_pAddBlendState);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_RASTERIZER_DESC descRast = {
		D3D11_FILL_SOLID,
		D3D11_CULL_FRONT,
		FALSE,
		D3D11_DEFAULT_DEPTH_BIAS,
		D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		FALSE,
		FALSE,
		FALSE,
		FALSE
	};
	descRast.CullMode = D3D11_CULL_FRONT;
	hr = pD3D11Device->CreateRasterizerState(&descRast, &m_pNoDepthClipFrontRS);
	if (FAILED(hr))
	{
		return hr;
	}
	D3D11_RASTERIZER_DESC rasterDesc;
	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	hr = pD3D11Device->CreateRasterizerState(&rasterDesc, &m_RSCullBack);
	if (FAILED(hr))
		return hr;
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	hr = pD3D11Device->CreateRasterizerState(&rasterDesc, &m_RSCullFont);
	if (FAILED(hr))
		return hr;


	m_record = new D3D11TextureRecord();
	hr = m_record->Initial(pDevice, pLightParameter->width,pLightParameter->height);
	if (FAILED(hr))
	{
		return hr;
	}
	// Create gBuffer constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CB_GBUFFER_UNPACK);
	hr = pD3D11Device->CreateBuffer(&cbDesc, NULL, &m_pGBufferUnpackCB);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 0;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pD3D11Device->CreateSamplerState(&sampDesc, &m_clampSample);

	if (FAILED(hr))
	{
		return hr;
	}
	
	return S_OK;
}

void D3D11LightRenderManager::Render(DXInF* pDevice, Parameter* pParameter)
{
	ID3D11DeviceContext* pD3D11DeviceContext = ((D3D11Class*)pDevice)->GetDeviceContext();
	
	LightRenderParameter* pRenderParameter = (LightRenderParameter*)pParameter;

	//update data for unpack Gbuffer
	PrepareForUnpack(pD3D11DeviceContext, pRenderParameter->pCamera);
	
	
	ID3D11BlendState* pPrevBlendState = NULL;
	FLOAT prevBlendFactor[4];
	UINT prevSampleMask;
	pD3D11DeviceContext->OMGetBlendState(&pPrevBlendState, prevBlendFactor, &prevSampleMask);
	pD3D11DeviceContext->OMSetBlendState(m_pAdditiveBlendState, prevBlendFactor, prevSampleMask);

	ID3D11RasterizerState* prevRS;
	pD3D11DeviceContext->RSGetState(&prevRS);
	pD3D11DeviceContext->RSSetState(m_RSCullBack);


	// Store the previous depth state
	ID3D11DepthStencilState* pPrevDepthState;
	UINT nPrevStencil;
	pD3D11DeviceContext->OMGetDepthStencilState(&pPrevDepthState, &nPrevStencil);
	
	m_record->BeginRecord(pD3D11DeviceContext, true, true);

	//first we have to do ambient Light
	AmbientLightRenderParameter ambientParameter;
	ambientParameter.colorSRV = pRenderParameter->colorSRV;
	ambientParameter.depthStencilDSV = pRenderParameter->depthStencilDSV;
	ambientParameter.normalSRV = pRenderParameter->normalSRV;
	ambientParameter.specPowerSRV = pRenderParameter->specPowerSRV;
	ambientParameter.GBufferUnpackCB = m_pGBufferUnpackCB;
	ambientParameter.tranparent = pRenderParameter->transparent;
	//hard code
	ambientParameter.LowerLight = XMFLOAT3(0.2f, 0.2f, 0.2f);
	ambientParameter.upperLight = XMFLOAT3(0.2f, 0.2f, 0.2f);
	pD3D11DeviceContext->OMSetDepthStencilState(m_pDepthWriteLessStencilMaskState, 1);
	m_pAmbientLight->Render(pD3D11DeviceContext, NULL, NULL, &ambientParameter);
	//do other light source
	for (unsigned int i = 0; i < pRenderParameter->pLights->size(); i++)
	{
		if (DirectXHelper::instanceof<DirectionLightObj>(pRenderParameter->pLights->operator[](i)))
		{
			//set voxel Light render pass
			pD3D11DeviceContext->PSSetConstantBuffers(VOXEL_LIGHT_RENDER_CB_INDEX, 1, &pRenderParameter->voxelLightRenderCB);
			pD3D11DeviceContext->PSSetShaderResources(VOXEL_LIGHT_SRV_INDEX, 1, &pRenderParameter->voxelLightPassSRV);
			pD3D11DeviceContext->PSSetSamplers(VOXEL_CLAMP_SAMPLER, 1, &m_clampSample);
			// Set the depth state for the directional light
			pD3D11DeviceContext->OMSetDepthStencilState(m_pNoDepthWriteGreatherStencilMaskState, 1);
			pD3D11DeviceContext->RSSetState(m_RSCullBack);
			DirectionalLightRenderParameter directionalRenderParameter;
			directionalRenderParameter.GBufferUnpackCB = m_pGBufferUnpackCB;
			directionalRenderParameter.colorSRV = pRenderParameter->colorSRV;
			directionalRenderParameter.depthStencilDSV = pRenderParameter->depthStencilDSV;
			directionalRenderParameter.normalSRV = pRenderParameter->normalSRV;
			directionalRenderParameter.specPowerSRV = pRenderParameter->specPowerSRV;
			directionalRenderParameter.transparent = pRenderParameter->transparent;
			if (pRenderParameter->shadowManager != NULL)
			{
				directionalRenderParameter.shadow = (DirectionalLightSahdow*)pRenderParameter->shadowManager->GetShadowData((int)pRenderParameter->pLights->operator[](i));
			}
			else
			{
				directionalRenderParameter.shadow = NULL;
			}
			m_pDirectionLight->Render(pD3D11DeviceContext, pRenderParameter->pLights->operator[](i), pRenderParameter->pCamera, &directionalRenderParameter);
			ID3D11Buffer* nullBuffer = NULL;
			pD3D11DeviceContext->PSSetConstantBuffers(VOXEL_LIGHT_RENDER_CB_INDEX, 1, &nullBuffer);
			ID3D11ShaderResourceView* nullResource = NULL;
			pD3D11DeviceContext->PSSetShaderResources(VOXEL_LIGHT_SRV_INDEX, 1, &nullResource);
			ID3D11SamplerState* nullSample = NULL;
			pD3D11DeviceContext->PSSetSamplers(VOXEL_CLAMP_SAMPLER, 1, &nullSample);
		}
		else if (DirectXHelper::instanceof<PointLightObj>(pRenderParameter->pLights->operator[](i)))
		{
			pD3D11DeviceContext->OMSetDepthStencilState(m_pNoDepthWriteGreatherStencilMaskState, 1);
			pD3D11DeviceContext->RSSetState(m_RSCullFont);
			PointLightRenderParameter pointRenderParameter;
			pointRenderParameter.GBufferUnpackCB = m_pGBufferUnpackCB;
			pointRenderParameter.colorSRV = pRenderParameter->colorSRV;
			pointRenderParameter.depthStencilDSV = pRenderParameter->depthStencilDSV;
			pointRenderParameter.normalSRV = pRenderParameter->normalSRV;
			pointRenderParameter.specPowerSRV = pRenderParameter->specPowerSRV;
			pointRenderParameter.transparent = pRenderParameter->transparent;
			m_pPointLight->Render(pD3D11DeviceContext, pRenderParameter->pLights->operator[](i), pRenderParameter->pCamera, &pointRenderParameter);
		}
		else if (DirectXHelper::instanceof<SpotLightObj>(pRenderParameter->pLights->operator[](i)))
		{
			pD3D11DeviceContext->OMSetDepthStencilState(m_pNoDepthWriteGreatherStencilMaskState, 1);
			pD3D11DeviceContext->RSSetState(m_RSCullFont);
			SpotLightRenderParameter spotRenderParameter;
			spotRenderParameter.GBufferUnpackCB = m_pGBufferUnpackCB;
			spotRenderParameter.colorSRV = pRenderParameter->colorSRV;
			spotRenderParameter.depthStencilDSV = pRenderParameter->depthStencilDSV;
			spotRenderParameter.normalSRV = pRenderParameter->normalSRV;
			spotRenderParameter.specPowerSRV = pRenderParameter->specPowerSRV;
			spotRenderParameter.isTransparent = pRenderParameter->transparent;
			m_pSpotLight->Render(pD3D11DeviceContext, pRenderParameter->pLights->operator[](i), pRenderParameter->pCamera, &spotRenderParameter);
		}
		
	}
	m_record->EndRecord(pD3D11DeviceContext);

	// Once we are done with the directional light, turn on the blending
	// Restore the states
	pD3D11DeviceContext->OMSetDepthStencilState(pPrevDepthState, nPrevStencil);
	if (pPrevDepthState) 
	{
		pPrevDepthState->Release();
	}
	pD3D11DeviceContext->OMSetBlendState(pPrevBlendState, prevBlendFactor, prevSampleMask);
	if (pPrevBlendState) 
	{
		pPrevBlendState->Release();
	}
	pD3D11DeviceContext->RSSetState(prevRS);
	if (prevRS)
	{
		prevRS->Release();
	}
}

void D3D11LightRenderManager::Update(DXInF* pDevice, Parameter* pParameter)
{

}

void D3D11LightRenderManager::Destroy()
{
	if (m_pDirectionLight)
	{
		delete m_pDirectionLight;
		m_pDirectionLight = NULL;
	}
	if (m_pAmbientLight)
	{
		delete m_pAmbientLight;
		m_pAmbientLight = NULL;
	}
	if (m_pPointLight)
	{
		delete m_pPointLight;
		m_pPointLight = NULL;
	}
	if (m_record)
	{
		delete m_record;
		m_record = NULL;
	}
	if (m_pSpotLight)
	{
		delete m_pSpotLight;
		m_pSpotLight = NULL;
	}
	SAFE_RELEASE(m_pDepthWriteLessStencilMaskState);
	SAFE_RELEASE(m_pNoDepthWriteGreatherStencilMaskState);
	SAFE_RELEASE(m_pAdditiveBlendState);
	SAFE_RELEASE(m_pAddBlendState);
	SAFE_RELEASE(m_pNoDepthClipFrontRS);
	SAFE_RELEASE(m_pGBufferUnpackCB);
	SAFE_RELEASE(m_RSCullBack);
	SAFE_RELEASE(m_RSCullFont);
	SAFE_RELEASE(m_clampSample);
}
void D3D11LightRenderManager::PrepareForUnpack(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera)
{
	// Fill the GBuffer unpack constant buffer
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dImmediateContext->Map(m_pGBufferUnpackCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CB_GBUFFER_UNPACK* pGBufferUnpackCB = (CB_GBUFFER_UNPACK*)MappedResource.pData;
	const XMMATRIX pProj = camera->GetProjection();
	XMFLOAT4X4 fProj;
	XMStoreFloat4x4(&fProj, pProj);
	pGBufferUnpackCB->PerspectiveValues.x = 1.0f / fProj._11;
	pGBufferUnpackCB->PerspectiveValues.y = 1.0f / fProj._22;
	pGBufferUnpackCB->PerspectiveValues.z = fProj._43;
	pGBufferUnpackCB->PerspectiveValues.w = -fProj._33;

	XMMATRIX matViewInv;
	DirectXHelper::InverseMatrix(camera->GetView(), matViewInv);
	pGBufferUnpackCB->ViewInv = XMMatrixTranspose(matViewInv);
	pd3dImmediateContext->Unmap(m_pGBufferUnpackCB, 0);
}
ID3D11ShaderResourceView* D3D11LightRenderManager::GetLightSRV()
{
	return ((D3D11TextureRecord*)m_record)->GetTexture();
}