#include "DXInclude.h"
#include "D3D11VoxelizationThread.h"
#include "D3D11Class.h"
#include "D3D11Model.h"
#include "ObjManager.h"
#include "FunctionHelper.h"
#include "D3D11DirectionalLightRender.h"
#include "FunctionHelper.h"
#define VOXEL_TEXTURE_SIZE	128
#define VOXEL_FILE "Data/Shader/VoxelizationGBuffer.fx"
#define VOXEL_LIGHT_INJECTION_FILE "Data/Shader/VoxelInjectRadiance.fx"
#define CB_SLOT	4

//inject radiance
#define VOXEL_INJECT_RADIANCE 3
#define VOXEL_INJECT_RADIANCE_ALBEDO 0
#define VOXEL_INJECT_RADIANCE_NORMAL 1
#define VOXEL_INJECT_RADIANCE_EMISSION 2
#define VOXEL_INJECT_RADIANCE_LIGHTPASS 0
#define VOXEL_INJECT_RADIANCE_DIR_LIGHT 2

#define VOXEL_DIRECTIONAL_LIGHT_MAX	50

const float CLEAR_RENDER[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
const UINT8 STENCIL_VALUE = 1;
const UINT8 STENCIL_CLEAR_VALUE = 0;



struct CBInjectRadiance
{
	XMFLOAT3 eyePosition;
	float pad0;
	XMFLOAT3 worldMinPoint;
	float pad1;
	float voxelSize;
	float voxelScale;
	UINT volumeDimension;
	float pad2;
};

struct CB_GBUFFER_DIM
{
	XMMATRIX VP[3];
	XMMATRIX VPI[3];
	XMFLOAT3 worldMinPoint;
	float voxelScale;
	UINT volumeDimension;
	float pad[3];
};
struct CB_VOXEL_DIRECTIONAL
{
	XMFLOAT3 vDirToLight;
	float intensity;
	XMFLOAT4 vDirectionalColor;
	
};

struct CBLightingResource
{
	CB_VOXEL_DIRECTIONAL directionLight[VOXEL_DIRECTIONAL_LIGHT_MAX];
};

D3D11VoxelizationThread::D3D11VoxelizationThread() :
	m_voxelCB(NULL),
	m_voxelColorRT(NULL),
	m_voxelNormalRT(NULL),
	m_voxelSpecPowerRT(NULL),
	m_voxelColorSRV(NULL),
	m_voxelNormalSRV(NULL),
	m_voxelSpecPowerSRV(NULL),
	m_voxelColorUAV(NULL),
	m_voxelNormalUAV(NULL),
	m_voxelSpecUAV(NULL),
	m_voxelLightPassRT(NULL),
	m_voxelLightPassUAV(NULL),
	m_voxelLightPassSRV(NULL),
	m_voxelColorRTV(NULL),
	m_voxelLightPassRTV(NULL),
	m_voxelNormalRTV(NULL),
	m_voxelSpecRTV(NULL), 
	m_dummyRTV(NULL),
	m_DepthStencilDSV(NULL),
	m_DepthStencilRT(NULL),
	m_dummyRT(NULL),
	m_RSCullBack(NULL),
	m_DepthStencilState(NULL),
	m_voxelInjectRadianceCB(NULL)
{
	
}
D3D11VoxelizationThread::~D3D11VoxelizationThread()
{
	Destroy();
}

HRESULT D3D11VoxelizationThread::Initial(DXInF* pDevice, Parameter* pParameter)
{
	m_Vp.MinDepth = 0.f;
	m_Vp.MaxDepth = 1.f;
	m_Vp.TopLeftX = 0.f;
	m_Vp.TopLeftY = 0.f;
	m_Vp.Width = (float)VOXEL_TEXTURE_SIZE;
	m_Vp.Height = (float)VOXEL_TEXTURE_SIZE;

	HRESULT hr = S_FALSE;
	//transfer data first before send to super data
	RenderThreadInitialParameter superClassParameter;
	superClassParameter.width = VOXEL_TEXTURE_SIZE;
	superClassParameter.height = VOXEL_TEXTURE_SIZE;
	hr = D3D11RenderThread::Initial(pDevice, &superClassParameter);
	if (FAILED(hr))
	{
		return hr;
	}
	ID3D11Device* device = ((D3D11Class*)pDevice)->GetDevice();

	m_projection = XMMatrixPerspectiveFovLH(0.25f*DirectX::XM_PI, (float)VOXEL_TEXTURE_SIZE / (float)VOXEL_TEXTURE_SIZE, 1.0f, 1000.0f);
	
	m_volumeDimension = VOXEL_TEXTURE_SIZE;
	D3D11_TEXTURE3D_DESC desc;
	desc.Width = VOXEL_TEXTURE_SIZE;
	desc.Height = VOXEL_TEXTURE_SIZE;
	desc.Depth = VOXEL_TEXTURE_SIZE;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_DEFAULT;
	hr = device->CreateTexture3D(&desc, NULL, &m_voxelColorRT);
	if (FAILED(hr)) 
	{
		return hr;
	}
	desc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	hr = device->CreateTexture3D(&desc, NULL, &m_voxelNormalRT);
	if (FAILED(hr))
	{
		return hr;
	}
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	hr = device->CreateTexture3D(&desc, NULL, &m_voxelSpecPowerRT);
	if (FAILED(hr))
	{
		return hr;
	}
	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	desc.MipLevels = 6;
	hr = device->CreateTexture3D(&desc, NULL, &m_voxelLightPassRT);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create a shader resource view to the voxel.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(m_voxelColorRT, &srvDesc, &m_voxelColorSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	srvDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	hr = device->CreateShaderResourceView(m_voxelNormalRT, &srvDesc, &m_voxelNormalSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	hr = device->CreateShaderResourceView(m_voxelSpecPowerRT, &srvDesc, &m_voxelSpecPowerSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	srvDesc.Texture3D.MipLevels = 6;
	hr = device->CreateShaderResourceView(m_voxelLightPassRT, &srvDesc, &m_voxelLightPassSRV);
	if (FAILED(hr))
	{
		return hr;
	}

	// Render view formats
	static const DXGI_FORMAT depthStencilRenderViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	static const DXGI_FORMAT basicColorRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT normalRenderViewFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	static const DXGI_FORMAT specPowRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT LightPassViewFormat = DXGI_FORMAT_R16G16B16A16_UNORM;

	D3D11_RENDER_TARGET_VIEW_DESC rtsvd =
	{
		basicColorRenderViewFormat,
		D3D11_RTV_DIMENSION_TEXTURE3D,
		0,
		0
	};
	rtsvd.Texture3D.MipSlice = 0;
	rtsvd.Texture3D.FirstWSlice = 0;
	rtsvd.Texture3D.WSize = VOXEL_TEXTURE_SIZE;
	hr = device->CreateRenderTargetView(m_voxelColorRT, &rtsvd, &m_voxelColorRTV);
	if (FAILED(hr)) 
	{
		return hr;
	}

	rtsvd.Format = normalRenderViewFormat;
	hr = device->CreateRenderTargetView(m_voxelNormalRT, &rtsvd, &m_voxelNormalRTV);
	if (FAILED(hr))
	{
		return hr;
	}
	rtsvd.Format = specPowRenderViewFormat;
	hr = device->CreateRenderTargetView(m_voxelSpecPowerRT, &rtsvd, &m_voxelSpecRTV);
	if (FAILED(hr))
	{
		return hr;
	}
	rtsvd.Format = LightPassViewFormat;
	hr = device->CreateRenderTargetView(m_voxelLightPassRT, &rtsvd, &m_voxelLightPassRTV);
	if (FAILED(hr))
	{
		return hr;
	}
	//create depth
	
	// Allocate the depth stencil target
	D3D11_TEXTURE2D_DESC dtd = {
		VOXEL_TEXTURE_SIZE, //UINT Width;
		VOXEL_TEXTURE_SIZE, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		DXGI_FORMAT_UNKNOWN, //DXGI_FORMAT Format;
		1, //DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	dtd.Format = DXGI_FORMAT_R24G8_TYPELESS;
	hr = device->CreateTexture2D(&dtd, NULL, &m_DepthStencilRT);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create the render target views
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd =
	{
		depthStencilRenderViewFormat,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0
	};

	hr = device->CreateDepthStencilView(m_DepthStencilRT, &dsvd, &m_DepthStencilDSV);
	
	if (FAILED(hr))
	{
		return hr;
	}
	//create dummy Texture
	dtd.BindFlags = D3D11_BIND_RENDER_TARGET;
	dtd.Format = basicColorRenderViewFormat;
	
	hr = device->CreateTexture2D(&dtd, NULL, &m_dummyRT);
	if (FAILED(hr))
	{
		return hr;
	}
	// Create dummy RTV
	ZeroMemory(&rtsvd,sizeof(rtsvd));
	rtsvd =
	{
		basicColorRenderViewFormat,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};
	hr = device->CreateRenderTargetView(m_dummyRT, &rtsvd, &m_dummyRTV);
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
	//initial voxelization shader
	hr = m_voxelShader.Initial(pDevice, (char*)VOXEL_FILE, &shaderLayout, SHADER_MODE::VS_PS_GS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}

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
	descDepth.DepthFunc = D3D11_COMPARISON_LESS;
	descDepth.StencilEnable = TRUE;
	descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp = { D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS };
	descDepth.FrontFace = stencilMarkOp;
	descDepth.BackFace = stencilMarkOp;
	hr = device->CreateDepthStencilState(&descDepth, &m_DepthStencilState);
	if (FAILED(hr))
	{
		return hr;
	}
	
	hr = m_mvp.Initial(pDevice);
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
	hr = device->CreateUnorderedAccessView(m_voxelColorRT, &uavDesc, &m_voxelColorUAV);
	if (FAILED(hr))
	{
		return hr;
	}
	uavDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	hr = device->CreateUnorderedAccessView(m_voxelNormalRT, &uavDesc, &m_voxelNormalUAV);
	if (FAILED(hr))
	{
		return hr;
	}
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	hr = device->CreateUnorderedAccessView(m_voxelSpecPowerRT, &uavDesc, &m_voxelSpecUAV);
	if (FAILED(hr))
	{
		return hr;
	}
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	hr = device->CreateUnorderedAccessView(m_voxelLightPassRT, &uavDesc, &m_voxelLightPassUAV);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CB_GBUFFER_DIM);
	hr = device->CreateBuffer(&cbDesc, NULL, &m_voxelCB);
	if (FAILED(hr))
	{
		return hr;
	}
	// Create voxel inject radianuce constant buffers
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CBInjectRadiance);
	hr = device->CreateBuffer(&cbDesc, NULL, &m_voxelInjectRadianceCB);
	if (FAILED(hr))
	{
		return hr;
	}
	// Create voxel inject radianuce constant buffers
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CBLightingResource);
	hr = device->CreateBuffer(&cbDesc, NULL, &m_voxelLightResourceCB);
	if (FAILED(hr))
	{
		return hr;
	}

	//intial light injection shader
	hr = m_LightInjection.Initial(pDevice, (char*)VOXEL_LIGHT_INJECTION_FILE, NULL, SHADER_MODE::CS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void D3D11VoxelizationThread::Render(DXInF* pDevice, Parameter* pParameter)
{
	D3D11RenderThread::Render(pDevice, pParameter);
	GenerateVoxel();
	ComputeLightInjection();
}

void D3D11VoxelizationThread::Update(DXInF* pDevice, Parameter* pParameter)
{
	D3D11RenderThread::Update(pDevice, pParameter);
}

void D3D11VoxelizationThread::Destroy()
{
	D3D11RenderThread::Destroy();
	SAFE_RELEASE(m_voxelCB);
	SAFE_RELEASE(m_voxelColorRT);
	SAFE_RELEASE(m_voxelNormalRT);
	SAFE_RELEASE(m_voxelSpecPowerRT);
	SAFE_RELEASE(m_voxelColorSRV);
	SAFE_RELEASE(m_voxelNormalSRV);
	SAFE_RELEASE(m_voxelSpecPowerSRV);
	SAFE_RELEASE(m_voxelColorUAV);
	SAFE_RELEASE(m_voxelNormalUAV);
	SAFE_RELEASE(m_voxelSpecUAV);
	SAFE_RELEASE(m_voxelLightPassRT);
	SAFE_RELEASE(m_voxelLightPassUAV);
	SAFE_RELEASE(m_voxelLightPassSRV);
	SAFE_RELEASE(m_voxelColorRTV);
	SAFE_RELEASE(m_voxelLightPassRTV);
	SAFE_RELEASE(m_voxelNormalRTV);
	SAFE_RELEASE(m_voxelSpecRTV);
	SAFE_RELEASE(m_dummyRTV);
	SAFE_RELEASE(m_DepthStencilDSV);
	SAFE_RELEASE(m_DepthStencilRT);
	SAFE_RELEASE(m_dummyRT);
	SAFE_RELEASE(m_RSCullBack);
	SAFE_RELEASE(m_DepthStencilState);
	SAFE_RELEASE(m_voxelLightResourceCB);
	SAFE_RELEASE(m_voxelInjectRadianceCB);
	m_voxelShader.Destroy();
	m_mvp.Destroy();
	if (m_RenderParameter)
	{
		delete m_RenderParameter;
		m_RenderParameter = NULL;
	}
	m_LightInjection.Destroy();
	int a =0;
}
void D3D11VoxelizationThread::ThreadExcecute()
{
	while (true)
	{
		WaitForSingleObject(m_beginThread, INFINITE);
		Render(NULL, NULL);
		m_deviceContext->FinishCommandList(false, &m_commanList);
		SetEvent(m_endThread);
	}
}
void D3D11VoxelizationThread::RenderObj()
{

	for (unsigned int i = 0; i < m_RenderParameter->m_modelDataList->size(); i++)
	{
		ModelInF* pModelInfo = m_RenderParameter->m_modelDataList->operator[](i);

		if (m_RenderParameter->m_modelObjectList->count(pModelInfo->GetModelIndex().c_str()) == 0)
		{
			continue;
		}
		D3DModelInF* pModel = m_RenderParameter->m_modelObjectList->operator[](pModelInfo->GetModelIndex().c_str());

		D3D11ModelParameterRender* pRenderParameter = new D3D11ModelParameterRender();
		pRenderParameter->pCamera = m_RenderParameter->pCamera;
		pRenderParameter->pModelInfo = pModelInfo;
		pRenderParameter->pMVP = &m_mvp;
		pModel->Render(m_deviceContext, pRenderParameter);
		delete pRenderParameter;
	}
}
void D3D11VoxelizationThread::SetGBufferRenderParameter(LightManager* pLightManager, ObjScene* pParameter, Camera* camera)
{
	if (m_RenderParameter == NULL)
	{
		m_RenderParameter = new VoxelRenderParameter();
	}
	ObjManager* obj = dynamic_cast<ObjManager*>(pParameter);

	m_RenderParameter->m_modelDataList = obj->GetModelDataList();
	m_RenderParameter->m_modelObjectList = obj->GetModelObjectList();
	m_RenderParameter->pLightManager = pLightManager;
	m_RenderParameter->pCamera = camera;
}
void D3D11VoxelizationThread::GenerateVoxel()
{
	D3D11_VIEWPORT prevVp;
	UINT prevViewPortNumber = 1;
	//get prev View port
	m_deviceContext->RSGetViewports(&prevViewPortNumber, &prevVp);
	//clear all Texture
	m_deviceContext->ClearRenderTargetView(m_dummyRTV, CLEAR_RENDER);
	m_deviceContext->ClearRenderTargetView(m_voxelColorRTV, CLEAR_RENDER);
	m_deviceContext->ClearRenderTargetView(m_voxelNormalRTV, CLEAR_RENDER);
	m_deviceContext->ClearRenderTargetView(m_voxelSpecRTV, CLEAR_RENDER);
	m_deviceContext->ClearDepthStencilView(m_DepthStencilDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, STENCIL_CLEAR_VALUE);
	UpdateVoxelCB();
	m_deviceContext->GSSetConstantBuffers(CB_SLOT, 1, &m_voxelCB);
	m_deviceContext->PSSetConstantBuffers(CB_SLOT, 1, &m_voxelCB);
	ID3D11RasterizerState* prevState = NULL;
	m_deviceContext->RSGetState(&prevState);
	m_deviceContext->RSSetState(m_RSCullBack);
	//bind shader
	m_voxelShader.PreRender(m_deviceContext);
	ID3D11UnorderedAccessView* uav[3] = { m_voxelColorUAV ,m_voxelNormalUAV ,m_voxelSpecUAV };
	m_deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_dummyRTV, m_DepthStencilDSV, 3, 3, uav, NULL);
	m_deviceContext->OMSetDepthStencilState(m_DepthStencilState, STENCIL_VALUE);
	//set view port
	m_deviceContext->RSSetViewports(1, &m_Vp);
	//draw object
	RenderObj();
	m_voxelShader.PostRender(m_deviceContext);

	ID3D11DepthStencilView* nullDSV = NULL;
	ID3D11RenderTargetView* nullRT = NULL;
	m_deviceContext->OMSetRenderTargets(1, &nullRT, nullDSV);
	m_deviceContext->RSSetViewports(prevViewPortNumber, &prevVp);
	m_deviceContext->RSSetState(prevState);
	ID3D11Buffer* nullCB = NULL;
	m_deviceContext->GSSetConstantBuffers(CB_SLOT, 1, &nullCB);
	m_deviceContext->PSSetConstantBuffers(CB_SLOT, 1, &nullCB);
	ID3D11UnorderedAccessView* nullUAV[3] = { NULL, NULL, NULL };
	const UINT count = 0;
	ID3D11RenderTargetView* nullRTV = NULL;
	m_deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &nullRTV, NULL, 3, 3, nullUAV, count);
}
void D3D11VoxelizationThread::UpdateVoxelCB()
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_deviceContext->Map(m_voxelCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CB_GBUFFER_DIM* cb = (CB_GBUFFER_DIM*)MappedResource.pData;
	m_volumeGridSize = VOXEL_TEXTURE_SIZE;
	m_voxelSize = m_volumeGridSize / m_volumeDimension;
	auto halfSize = m_volumeGridSize / 2.0f;
	const auto &vCenter = m_RenderParameter->pCamera->GetPosition();
	XMFLOAT3 fCenter;
	DirectX::XMStoreFloat3(&fCenter,vCenter);

	// projection matrices
	auto projection = XMMatrixOrthographicOffCenterLH(-halfSize, halfSize, -halfSize, halfSize, 0.0f,
		m_volumeGridSize);
	// view matrices
	m_viewProjectionMatrix[0] = XMMatrixLookAtLH(XMLoadFloat3(&fCenter) + XMVectorSet(halfSize, 0.0f, 0.0f, 0.0f),
		XMLoadFloat3(&fCenter), XMVectorSet(0.0f, 1.0f, 0.0f, 0.f));
	m_viewProjectionMatrix[1] = XMMatrixLookAtLH(XMLoadFloat3(&fCenter) + XMVectorSet(0.0f, halfSize, 0.0f, 0.0f),
		XMLoadFloat3(&fCenter), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	m_viewProjectionMatrix[2] = XMMatrixLookAtLH(XMLoadFloat3(&fCenter) + XMVectorSet(0.0f, 0.0f, halfSize, 0.0f),
		XMLoadFloat3(&fCenter), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	int i = 0;

	for (auto &matrix : m_viewProjectionMatrix)
	{
		matrix = matrix * projection;
		XMMATRIX result;
		DirectXHelper::InverseMatrix(matrix, result);
		m_viewProjectionMatrixI[i++] = result;
	}

	cb->volumeDimension = m_volumeDimension;
	cb->voxelScale = 1.0f / m_volumeGridSize;
	cb->VP[0] = XMMatrixTranspose(m_viewProjectionMatrix[0]);
	cb->VP[1] = XMMatrixTranspose(m_viewProjectionMatrix[1]);
	cb->VP[2] = XMMatrixTranspose(m_viewProjectionMatrix[2]);
	cb->VPI[0] = XMMatrixTranspose(m_viewProjectionMatrixI[0]);
	cb->VPI[1] = XMMatrixTranspose(m_viewProjectionMatrixI[1]);
	cb->VPI[2] = XMMatrixTranspose(m_viewProjectionMatrixI[2]);
	cb->worldMinPoint = XMFLOAT3(fCenter.x - halfSize, fCenter.y - halfSize, fCenter.z - halfSize);
	m_deviceContext->Unmap(m_voxelCB, 0);

}
void D3D11VoxelizationThread::ComputeLightInjection()
{
	m_LightInjection.PreRender(m_deviceContext);
	m_deviceContext->ClearRenderTargetView(m_voxelLightPassRTV, CLEAR_RENDER);
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_deviceContext->Map(m_voxelInjectRadianceCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CBInjectRadiance* cb = (CBInjectRadiance*)MappedResource.pData;
	XMStoreFloat3(&cb->eyePosition, m_RenderParameter->pCamera->GetPosition());
	cb->worldMinPoint = XMFLOAT3(-m_volumeGridSize / 2, -m_volumeGridSize / 2, -m_volumeGridSize / 2);
	cb->voxelSize = m_voxelSize;
	cb->voxelScale = 1.0f / m_volumeGridSize;
	cb->volumeDimension = m_volumeDimension;
	m_deviceContext->Unmap(m_voxelInjectRadianceCB, 0);

	//add light data into constant buffer
	D3D11_MAPPED_SUBRESOURCE lightMappedResource;
	m_deviceContext->Map(m_voxelLightResourceCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &lightMappedResource);
	CBLightingResource* cbLight = (CBLightingResource*)lightMappedResource.pData;

	std::vector<LightObjInF*>* pLights = m_RenderParameter->pLightManager->GetLightArray();
	for (unsigned int i = 0; i < pLights->size(); i++)
	{
		if (DirectXHelper::instanceof<DirectionLightObj>(pLights->operator[](i)))
		{
			cbLight->directionLight[i].vDirectionalColor = pLights->operator[](i)->Color;
			cbLight->directionLight[i].vDirToLight = ((DirectionLightObj*)pLights->operator[](i))->Direction;
			cbLight->directionLight[i].vDirToLight.x *= -1.f;
			cbLight->directionLight[i].vDirToLight.y *= -1.f;
			cbLight->directionLight[i].vDirToLight.z *= -1.f;
			cbLight->directionLight[i].intensity = pLights->operator[](i)->Intensity;
		}
	}
	m_deviceContext->Unmap(m_voxelLightResourceCB, 0);

	m_deviceContext->CSSetConstantBuffers(VOXEL_INJECT_RADIANCE_DIR_LIGHT, 1, &m_voxelLightResourceCB);
	m_deviceContext->CSSetConstantBuffers(VOXEL_INJECT_RADIANCE, 1, &m_voxelInjectRadianceCB);
	m_deviceContext->CSSetShaderResources(VOXEL_INJECT_RADIANCE_ALBEDO, 1, &m_voxelColorSRV);
	m_deviceContext->CSSetShaderResources(VOXEL_INJECT_RADIANCE_NORMAL, 1, &m_voxelNormalSRV);
	m_deviceContext->CSSetShaderResources(VOXEL_INJECT_RADIANCE_EMISSION, 1, &m_voxelSpecPowerSRV);
	m_deviceContext->CSSetUnorderedAccessViews(VOXEL_INJECT_RADIANCE_LIGHTPASS, 1, &m_voxelLightPassUAV, NULL);

	UINT numThreat = (UINT)std::ceil(m_volumeDimension / 8.0f);
	m_deviceContext->Dispatch(numThreat, numThreat, numThreat);

	ID3D11Buffer* pNullCB = NULL;
	m_deviceContext->CSSetConstantBuffers(VOXEL_INJECT_RADIANCE_DIR_LIGHT, 1, &pNullCB);
	m_deviceContext->CSSetConstantBuffers(VOXEL_INJECT_RADIANCE, 1, &pNullCB);
	ID3D11ShaderResourceView* pNULLSRV = NULL;
	m_deviceContext->CSSetShaderResources(VOXEL_INJECT_RADIANCE_ALBEDO, 1, &pNULLSRV);
	m_deviceContext->CSSetShaderResources(VOXEL_INJECT_RADIANCE_NORMAL, 1, &pNULLSRV);
	m_deviceContext->CSSetShaderResources(VOXEL_INJECT_RADIANCE_EMISSION, 1, &pNULLSRV);
	ID3D11UnorderedAccessView* pNULLUAV = NULL;
	m_deviceContext->CSSetUnorderedAccessViews(VOXEL_INJECT_RADIANCE_LIGHTPASS, 1, &pNULLUAV, NULL);

	m_LightInjection.PostRender(m_deviceContext);
}