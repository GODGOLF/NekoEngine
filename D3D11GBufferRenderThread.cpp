#include "DXInclude.h"
#include "D3D11GBufferRenderThread.h"
#include "D3D11Class.h"
#include "D3D11Model.h"
#include "ObjManager.h"

#define G_BUFFER_FILE "Data/Shader/GBuffer.fx"
const float CLEAR_RENDER[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
const UINT8 STENCIL_VALUE = 1;
const UINT8 STENCIL_CLEAR_VALUE = 0;

struct CB_GBUFFER_UNPACK
{
	XMFLOAT4 PerspectiveValues;
	XMMATRIX  ViewInv;
};
D3D11GBufferRenderThread::D3D11GBufferRenderThread() :
	m_pGBufferUnpackCB(NULL),
	m_DepthStencilRT(NULL),
	m_ColorSpecIntensityRT(NULL),
	m_NormalRT(NULL),
	m_SpecPowerRT(NULL),
	m_DepthStencilSRV(NULL),
	m_ColorSpecIntensitySRV(NULL),
	m_NormalSRV(NULL),
	m_SpecPowerSRV(NULL),
	m_DepthStencilState(NULL),
	m_ColorSpecIntensityRTV(NULL),
	m_NormalRTV(NULL),
	m_SpecPowerRTV(NULL),
	m_DepthStencilDSV(NULL),
	m_DepthStencilReadOnlyDSV(NULL),
	m_RenderParameter(NULL)
{

}
D3D11GBufferRenderThread::~D3D11GBufferRenderThread()
{
	Destroy();
}
HRESULT D3D11GBufferRenderThread::Initial(DXInF* pDevice, Parameter* pParameter)
{
	GBufferInitialParameter* parameter = (GBufferInitialParameter*)pParameter;
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
	

	m_Vp.Width = (float)parameter->width;
	m_Vp.Height = (float)parameter->height;

	ID3D11Device* device = ((D3D11Class*)pDevice)->GetDevice();
	HRESULT result;
	// Texture formats
	static const DXGI_FORMAT depthStencilTextureFormat = DXGI_FORMAT_R24G8_TYPELESS;
	static const DXGI_FORMAT basicColorTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	static const DXGI_FORMAT normalTextureFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	static const DXGI_FORMAT specPowTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Render view formats
	static const DXGI_FORMAT depthStencilRenderViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	static const DXGI_FORMAT basicColorRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	static const DXGI_FORMAT normalRenderViewFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	static const DXGI_FORMAT specPowRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Resource view formats
	static const DXGI_FORMAT depthStencilResourceViewFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	static const DXGI_FORMAT basicColorResourceViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	static const DXGI_FORMAT normalResourceViewFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	static const DXGI_FORMAT specPowResourceViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Allocate the depth stencil target
	D3D11_TEXTURE2D_DESC dtd = {
		(UINT)parameter->width, //UINT Width;
		(UINT)parameter->height, //UINT Height;
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
	dtd.Format = depthStencilTextureFormat;
	result = device->CreateTexture2D(&dtd, NULL, &m_DepthStencilRT);
	if (FAILED(result)) {
		return result;
	}
	// Allocate the base color with specular intensity target
	dtd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	dtd.Format = basicColorTextureFormat;

	result = device->CreateTexture2D(&dtd, NULL, &m_ColorSpecIntensityRT);
	if (FAILED(result)) {
		return result;
	}
	// Allocate the base color with specular intensity target
	dtd.Format = normalTextureFormat;
	result = device->CreateTexture2D(&dtd, NULL, &m_NormalRT);
	if (FAILED(result)) {
		return result;
	}


	// Allocate the specular power target
	dtd.Format = specPowTextureFormat;
	result = device->CreateTexture2D(&dtd, NULL, &m_SpecPowerRT);
	if (FAILED(result)) {
		return result;
	}

	// Create the render target views
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd =
	{
		depthStencilRenderViewFormat,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0
	};

	result = device->CreateDepthStencilView(m_DepthStencilRT, &dsvd, &m_DepthStencilDSV);
	if (FAILED(result)) {
		return result;
	}

	dsvd.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	result = device->CreateDepthStencilView(m_DepthStencilRT, &dsvd, &m_DepthStencilReadOnlyDSV);
	if (FAILED(result)) {
		return result;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtsvd =
	{
		basicColorRenderViewFormat,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};
	result = device->CreateRenderTargetView(m_ColorSpecIntensityRT, &rtsvd, &m_ColorSpecIntensityRTV);
	if (FAILED(result)) {
		return result;
	}
	rtsvd.Format = normalRenderViewFormat;
	result = device->CreateRenderTargetView(m_NormalRT, &rtsvd, &m_NormalRTV);
	if (FAILED(result)) {
		return result;
	}
	rtsvd.Format = specPowRenderViewFormat;
	result = device->CreateRenderTargetView(m_SpecPowerRT, &rtsvd, &m_SpecPowerRTV);
	if (FAILED(result)) {
		return result;
	}
	// Create the resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =
	{
		depthStencilResourceViewFormat,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	dsrvd.Texture2D.MipLevels = 1;
	result = device->CreateShaderResourceView(m_DepthStencilRT, &dsrvd, &m_DepthStencilSRV);
	if (FAILED(result)) {
		return result;
	}
	dsrvd.Format = basicColorResourceViewFormat;
	result = device->CreateShaderResourceView(m_ColorSpecIntensityRT, &dsrvd, &m_ColorSpecIntensitySRV);
	if (FAILED(result)) {
		return result;
	}
	dsrvd.Format = normalResourceViewFormat;
	result = device->CreateShaderResourceView(m_NormalRT, &dsrvd, &m_NormalSRV);
	if (FAILED(result)) {
		return result;
	}
	dsrvd.Format = specPowResourceViewFormat;
	result = device->CreateShaderResourceView(m_SpecPowerRT, &dsrvd, &m_SpecPowerSRV);
	if (FAILED(result)) {
		return result;
	}
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
	result = device->CreateDepthStencilState(&descDepth, &m_DepthStencilState);
	if (FAILED(result)) {
		return result;
	}
	// Create constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CB_GBUFFER_UNPACK);
	result = device->CreateBuffer(&cbDesc, NULL, &m_pGBufferUnpackCB);
	if (FAILED(result)) {
		return result;
	}

	result =  m_mvp.Initial(pDevice);
	if (FAILED(result)) {
		return result;
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
	m_GBufferShader.Initial(pDevice,(char*)G_BUFFER_FILE, &shaderLayout, SHADER_MODE::VS_PS_MODE);


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
	hr = device->CreateRasterizerState(&rasterDesc, &m_RSCullBack);
	if (FAILED(hr))
		return hr;


	return S_OK;
}

void D3D11GBufferRenderThread::Render(DXInF* pDevice, Parameter* pParameter)
{
	if (m_RenderParameter == NULL)
	{
		return;
	}
	D3D11RenderThread::Render(pDevice,pParameter);
	D3D11PrePostRender shaderRenderParameter;
	shaderRenderParameter.deviceContext = m_deviceContext;
	m_GBufferShader.PreRender(&shaderRenderParameter);
	m_deviceContext->ClearRenderTargetView(m_ColorSpecIntensityRTV, CLEAR_RENDER);
	m_deviceContext->ClearRenderTargetView(m_NormalRTV, CLEAR_RENDER);
	m_deviceContext->ClearRenderTargetView(m_SpecPowerRTV, CLEAR_RENDER);
	m_deviceContext->ClearDepthStencilView(m_DepthStencilDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, STENCIL_CLEAR_VALUE);
	ID3D11RenderTargetView* rt[3] = { m_ColorSpecIntensityRTV, m_NormalRTV, m_SpecPowerRTV };
	m_deviceContext->OMSetRenderTargets(3, rt, m_DepthStencilDSV);
	m_deviceContext->OMSetDepthStencilState(m_DepthStencilState, STENCIL_VALUE);
	ID3D11RasterizerState* prevState = NULL;
	m_deviceContext->RSGetState(&prevState);
	m_deviceContext->RSSetState(m_RSCullBack);
	D3D11_VIEWPORT prevVp;
	UINT prevViewPortNumber = 1;
	//get prev View port
	m_deviceContext->RSGetViewports(&prevViewPortNumber, &prevVp);
	//set view port
	m_deviceContext->RSSetViewports(1, &m_Vp);
	RenderObj(pDevice);
	m_GBufferShader.PostRender(&shaderRenderParameter);
	ID3D11RenderTargetView* nullRT[3] = { NULL, NULL, NULL };
	m_deviceContext->OMSetRenderTargets(3, nullRT, m_DepthStencilReadOnlyDSV);
	m_deviceContext->RSSetViewports(prevViewPortNumber, &prevVp);
	m_deviceContext->RSSetState(prevState);

}

void D3D11GBufferRenderThread::Update(DXInF* pDevice, Parameter* pParameter)
{
	D3D11RenderThread::Update(pDevice, pParameter);
	
	
}

void D3D11GBufferRenderThread::Destroy()
{
	D3D11RenderThread::Destroy();
	m_GBufferShader.Destroy();
	SAFE_RELEASE(m_pGBufferUnpackCB);
	SAFE_RELEASE(m_DepthStencilRT);
	SAFE_RELEASE(m_ColorSpecIntensityRT);
	SAFE_RELEASE(m_NormalRT);
	SAFE_RELEASE(m_SpecPowerRT);
	SAFE_RELEASE(m_DepthStencilDSV);
	SAFE_RELEASE(m_DepthStencilReadOnlyDSV);
	SAFE_RELEASE(m_ColorSpecIntensityRTV);
	SAFE_RELEASE(m_NormalRTV);
	SAFE_RELEASE(m_SpecPowerRTV);
	SAFE_RELEASE(m_DepthStencilSRV);
	SAFE_RELEASE(m_ColorSpecIntensitySRV);
	SAFE_RELEASE(m_NormalSRV);
	SAFE_RELEASE(m_SpecPowerSRV);
	SAFE_RELEASE(m_DepthStencilState);
	SAFE_RELEASE(m_RSCullBack);
	if (m_RenderParameter)
	{
		delete m_RenderParameter;
	}
	m_pDevice = NULL;
}

void D3D11GBufferRenderThread::ThreadExcecute()
{
	while (true)
	{
		WaitForSingleObject(m_beginThread, INFINITE);
		Render(m_pDevice, NULL);
		m_deviceContext->FinishCommandList(false, &m_commanList);
		SetEvent(m_endThread);
	}
	
}
void D3D11GBufferRenderThread::RenderObj(DXInF* pDevice)
{
	
	for (unsigned int i = 0; i < m_RenderParameter->m_modelDataList->size(); i++)
	{
		ModelInF* pModelInfo = m_RenderParameter->m_modelDataList->operator[](i);
		
		if (m_RenderParameter->m_modelObjectList->count(pModelInfo->modelIndex.c_str()) ==0)
		{
			continue;
		}
		D3DModelInF* pModel = m_RenderParameter->m_modelObjectList->operator[](pModelInfo->modelIndex.c_str());
		
		D3D11ModelParameterRender* pRenderParameter = new D3D11ModelParameterRender();
		pRenderParameter->pCamera = m_RenderParameter->pCamera;
		pRenderParameter->pDeviceContext = m_deviceContext;
		pRenderParameter->pModelInfo = pModelInfo;
		pRenderParameter->pMVP = &m_mvp;
		pModel->Render(pDevice, pRenderParameter);
	}
}
void D3D11GBufferRenderThread::SetGBufferRenderParameter(ObjScene* pParameter, Camera* camera)
{
	if (m_RenderParameter == NULL)
	{
		m_RenderParameter = new GBufferRenderParameter();
	}
	ObjManager* obj = dynamic_cast<ObjManager*>(pParameter);

	m_RenderParameter->m_modelDataList = obj->GetModelDataList();
	m_RenderParameter->m_modelObjectList = obj->GetModelObjectList();
	m_RenderParameter->pCamera = camera;
}
