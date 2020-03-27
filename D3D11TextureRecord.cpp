#include "DXInclude.h"
#include "D3D11TextureRecord.h"
#include "D3D11Class.h"

D3D11TextureRecord::D3D11TextureRecord() :
	m_prevRenderTarget(NULL), 
	m_prevDepthStencilView(NULL),
	m_depthStencilView(NULL),
	m_renderTargetView(NULL),
	m_shaderResourceView(NULL)
{

}
D3D11TextureRecord::~D3D11TextureRecord()
{
	Destroy();
}
HRESULT D3D11TextureRecord::Initial(DXInF* pDevice, int width, int height)
{
	ID3D11Device* pD3D11Device = ((D3D11Class*)pDevice)->GetDevice();
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	HRESULT hr;
	ID3D11Texture2D * textureMap;
	///////////////////////// Map's Texture
	// Initialize the  texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the texture description.
	// We will have our map be a square
	// We will need to have this texture bound as a render target AND a shader resource
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE |
		D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the texture
	hr = pD3D11Device->CreateTexture2D(&textureDesc, NULL, &textureMap);
	if (FAILED(hr)) {
		return hr;
	}

	/////////////////////// Map's Render Target
	// Setup the description of the render target view.
	renderTargetViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	hr = pD3D11Device->CreateRenderTargetView(textureMap, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(hr)) {
		return hr;
	}
	/////////////////////// Map's Shader Resource View
	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	hr = pD3D11Device->CreateShaderResourceView(textureMap, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(hr)) {
		return hr;
	}
	if (textureMap) {
		textureMap->Release();
		textureMap = NULL;
	}
	ID3D11Texture2D* depthStencilBuffer;
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
	hr = pD3D11Device->CreateTexture2D(&descDepth, nullptr, &depthStencilBuffer);
	if (FAILED(hr)) {
		return hr;
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = pD3D11Device->CreateDepthStencilView(depthStencilBuffer, &descDSV, &m_depthStencilView);
	if (FAILED(hr)) {
		return hr;
	}
	if (depthStencilBuffer) {
		depthStencilBuffer->Release();
		depthStencilBuffer = NULL;
	}
	// Setup the viewport
	m_vp.Width = (FLOAT)width;
	m_vp.Height = (FLOAT)height;
	m_vp.MinDepth = 0.0f;
	m_vp.MaxDepth = 1.0f;
	m_vp.TopLeftX = 0;
	m_vp.TopLeftY = 0;
	return S_OK;
}
void D3D11TextureRecord::BeginRecord(void* pDeviceContext, bool clearDepth, bool clearRenderTarget)
{
	ID3D11DeviceContext* pD3D11DeviceContext = (ID3D11DeviceContext*)pDeviceContext;
	//get prev render target and depth stencil view
	pD3D11DeviceContext->OMGetRenderTargets(1, &m_prevRenderTarget, &m_prevDepthStencilView);
	//get previous viewport
	UINT pNumViewPort = 1;
	pD3D11DeviceContext->RSGetViewports(&pNumViewPort, &m_prevVP);
	// Set our maps Render Target
	pD3D11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	pD3D11DeviceContext->RSSetViewports(1, &m_vp);
	// Now clear the render target
	XMVECTORF32 dark = { 0.0000000f, 0.000000f, 0.000000000f, 1.000000000f };
	if (clearRenderTarget) {
		pD3D11DeviceContext->ClearRenderTargetView(m_renderTargetView, dark);
	}
	if (clearDepth) {
		pD3D11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
}
void D3D11TextureRecord::EndRecord(void* pDeviceContext)
{
	ID3D11DeviceContext* pD3D11DeviceContext = (ID3D11DeviceContext*)pDeviceContext;
	ID3D11RenderTargetView* pNullRTV = NULL;
	pD3D11DeviceContext->OMSetRenderTargets(1, &pNullRTV, NULL);
	//restore prev renderTarget
	pD3D11DeviceContext->OMSetRenderTargets(1, &m_prevRenderTarget, m_prevDepthStencilView);
	pD3D11DeviceContext->RSSetViewports(1, &m_prevVP);
	if (m_prevRenderTarget) {
		m_prevRenderTarget->Release();
		m_prevRenderTarget = NULL;
	}
	if (m_prevDepthStencilView) {
		m_prevDepthStencilView->Release();
		m_prevDepthStencilView = NULL;
	}
}
void D3D11TextureRecord::Destroy()
{
	SAFE_RELEASE(m_prevRenderTarget);
	SAFE_RELEASE(m_prevDepthStencilView);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_shaderResourceView);
}
ID3D11ShaderResourceView* D3D11TextureRecord::GetTexture()
{
	return m_shaderResourceView;
}