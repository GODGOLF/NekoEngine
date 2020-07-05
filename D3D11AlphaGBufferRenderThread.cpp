#include"DXInclude.h"
#include "D3D11AlphaGBufferRenderThread.h"
#include "D3D11Class.h"
#define TRANSPARENT_LAYER	18

D3D11AlphaGBufferRenderThread::D3D11AlphaGBufferRenderThread()
{
	m_isTranparent = true;
}
D3D11AlphaGBufferRenderThread::~D3D11AlphaGBufferRenderThread()
{
	Destroy();
}
HRESULT D3D11AlphaGBufferRenderThread::Initial(DXInF* pDevice, Parameter* pParameter = nullptr)
{
	HRESULT hr = S_OK;
	AlphaGBufferInitialParameter* parameter = (AlphaGBufferInitialParameter*)pParameter;
	D3D11GBufferRenderThread::Initial(pDevice, pParameter);

	ID3D11Device* device = ((D3D11Class*)pDevice)->GetDevice();

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = parameter->width;
	desc.Height = parameter->height;
	desc.Depth = TRANSPARENT_LAYER;
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
	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	hr = device->CreateTexture3D(&desc, NULL, &m_depthLayerRT);
	if (FAILED(hr))
	{
		return hr;
	}


	return hr;
}

void D3D11AlphaGBufferRenderThread::Render(DXInF* pDevice, Parameter* pParameter = nullptr)
{
	D3D11GBufferRenderThread::Render(pDevice, pParameter);
	
}

void D3D11AlphaGBufferRenderThread::Update(DXInF* pDevice, Parameter* pParameter = nullptr)
{
	D3D11GBufferRenderThread::Update(pDevice, pParameter);
}

void D3D11AlphaGBufferRenderThread::Destroy()
{
	D3D11GBufferRenderThread::Destroy();
}

