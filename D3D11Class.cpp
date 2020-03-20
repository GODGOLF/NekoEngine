#include "DXInclude.h"
#include "D3D11Class.h"


D3D11Class::D3D11Class() 
{
	g_pd3dDevice = 0;
	g_pImmediateContext = 0;
	g_pSwapChain = 0;

	g_pRenderTargetView = 0;
	m_alphaEnableBlendingState = 0;
	m_alphaDisableBlendingState = 0;


	m_depthStencilBuffer = 0;
	m_depthStencilView = 0;
	m_depthStencilState = 0;
	m_RSCullBack = 0;
}
D3D11Class::~D3D11Class() {
	OnDestroy();
}
HRESULT D3D11Class::OnInit(HWND hwnd, UINT width, UINT height) 
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;


	// DirectX 11.0 systems
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
	if (FAILED(hr))
		return hr;


	dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;


	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);


	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;
	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &m_depthStencilBuffer);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(m_depthStencilBuffer, &descDSV, &m_depthStencilView);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, m_depthStencilView);


	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


	// Create the depth stencil state.
	hr = g_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(hr))
		return hr;

	//set depthStencil
	g_pImmediateContext->OMSetDepthStencilState(m_depthStencilState, 1);




	// Bind the render target view and depth stencil buffer to the output render pipeline.

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
	hr = g_pd3dDevice->CreateRasterizerState(&rasterDesc, &m_RSCullBack);
	if (FAILED(hr))
		return hr;

	// Now set the rasterizer state.
	g_pImmediateContext->RSSetState(m_RSCullBack);

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
	hr = g_pd3dDevice->CreateRasterizerState(&rasterDesc, &m_RSCullNone);
	if (FAILED(hr))
		return hr;

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	hr = g_pd3dDevice->CreateRasterizerState(&rasterDesc, &m_RSCullFront);
	if (FAILED(hr))
		return hr;


	// Setup the viewport
	m_vp.Width = (FLOAT)width;
	m_vp.Height = (FLOAT)height;
	m_vp.MinDepth = 0.0f;
	m_vp.MaxDepth = 1.0f;
	m_vp.TopLeftX = 0;
	m_vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &m_vp);

	//create blend
	D3D11_BLEND_DESC blendStateDescription;

	// Clear the blend state description.
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	//blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	//blendStateDescription.AlphaToCoverageEnable = true;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	hr = g_pd3dDevice->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState);
	if (FAILED(hr))
		return hr;
	blendStateDescription.AlphaToCoverageEnable = true;
	hr = g_pd3dDevice->CreateBlendState(&blendStateDescription, &m_alphaCoverEnableBlendingState);
	if (FAILED(hr))
		return hr;

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

	// Create the blend state using the description.
	hr = g_pd3dDevice->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState);
	if (FAILED(hr))
		return hr;




	return S_OK;
}
void D3D11Class::OnDestroy() 
{
	if (g_pRenderTargetView) {
		g_pRenderTargetView->Release();
		g_pRenderTargetView = 0;
	}
	if (g_pSwapChain) {
		g_pSwapChain->Release();
		g_pSwapChain = 0;
	}

	if (g_pImmediateContext) {
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Release();
		g_pImmediateContext = 0;
	}

	if (g_pd3dDevice) {
		g_pd3dDevice->Release();
		g_pd3dDevice = 0;

	}
	if (m_alphaEnableBlendingState) {
		m_alphaEnableBlendingState->Release();
		m_alphaEnableBlendingState = 0;
	};
	if (m_alphaDisableBlendingState) {
		m_alphaDisableBlendingState->Release();
		m_alphaDisableBlendingState = 0;
	};
	if (m_alphaCoverEnableBlendingState) {
		m_alphaCoverEnableBlendingState->Release();
		m_alphaCoverEnableBlendingState = 0;
	}
	if (m_depthStencilBuffer) {
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}
	if (m_depthStencilView) {
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}
	if (m_depthStencilState) {
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}
	if (m_RSCullBack) {
		m_RSCullBack->Release();
		m_RSCullBack = 0;
	}
	if (m_RSCullNone) {
		m_RSCullNone->Release();
		m_RSCullNone = 0;
	}
	if (m_RSCullFront) {
		m_RSCullFront->Release();
		m_RSCullFront = 0;
	}
}
void D3D11Class::BindMainRenderTarget() {
	Reset();
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, g_backgroundColor_);
	g_pImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, m_depthStencilView);
}
void D3D11Class::EndDraw() {
	g_pSwapChain->Present(1, 0);
}
ID3D11Device* D3D11Class::GetDevice() {
	return g_pd3dDevice;
}
ID3D11DeviceContext* D3D11Class::GetDeviceContext() {
	return g_pImmediateContext;
}

ID3D11RenderTargetView* D3D11Class::GetRenderTargetView() 
{
	return g_pRenderTargetView;
}
ID3D11DepthStencilView* D3D11Class::GetDepthStencilView() 
{
	return m_depthStencilView;
}
XMVECTORF32* D3D11Class::GetDefaultColorBg() 
{
	return &g_backgroundColor_;
}
void D3D11Class::Reset() 
{
	// Now set the rasterizer state.
	g_pImmediateContext->RSSetState(m_RSCullBack);
	g_pImmediateContext->OMSetDepthStencilState(m_depthStencilState, 1);
	g_pImmediateContext->RSSetViewports(1, &m_vp);
}

void D3D11Class::SetAlphaBlend(bool isTurnOn, bool alphaCover) {
	float blendFactor[4];

	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	if (isTurnOn) {
		if (alphaCover) {
			g_pImmediateContext->OMSetBlendState(m_alphaCoverEnableBlendingState, blendFactor, 0xffffffff);
		}
		else {
			// Turn on the alpha blending.
			g_pImmediateContext->OMSetBlendState(m_alphaEnableBlendingState, blendFactor, 0xffffffff);
		}

	}
	else {
		// Turn off the alpha blending.
		g_pImmediateContext->OMSetBlendState(m_alphaDisableBlendingState, blendFactor, 0xffffffff);
	}
}
void D3D11Class::SetCullRaterilization(D3D10_CULL_MODE mode) 
{
	switch (mode)
	{
	case D3D10_CULL_NONE:
		g_pImmediateContext->RSSetState(m_RSCullNone);
		break;
	case D3D10_CULL_FRONT:
		g_pImmediateContext->RSSetState(m_RSCullFront);
		break;
	case D3D10_CULL_BACK:
		g_pImmediateContext->RSSetState(m_RSCullBack);
		break;
	default:
		break;
	}
}