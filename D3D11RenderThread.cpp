#include "DXInclude.h"
#include "D3D11RenderThread.h"
#include "D3D11Class.h"

D3D11RenderThread::D3D11RenderThread() : m_pDevice(NULL)
{
}
D3D11RenderThread::~D3D11RenderThread()
{
	Destroy();
}
HRESULT D3D11RenderThread::Initial(DXInF* pDevice, Parameter* pParameter)
{
	HRESULT hr;
	D3D11Class* pDevice11 = (D3D11Class*)pDevice;

	RenderThreadInitialParameter* parameter = (RenderThreadInitialParameter*)pParameter;

	hr = pDevice11->GetDevice()->CreateDeferredContext(0, &m_deviceContext);

	if (!SUCCEEDED(hr)) {
		return hr;
	}

	m_defaultVP.Width = (float)parameter->width;
	m_defaultVP.Height = (float)parameter->height;

	m_deviceContext->RSSetViewports(1, &m_defaultVP);

	m_pDevice = pDevice;


	return S_OK;
}

void D3D11RenderThread::Render(DXInF* pDevice, Parameter* pParameter)
{

}

void D3D11RenderThread::Update(DXInF* pDevice, Parameter* pParameter)
{

}

void D3D11RenderThread::Destroy()
{
	SAFE_RELEASE(m_deviceContext);
}
void D3D11RenderThread::ThreadExcecute()
{
	
	
}
ID3D11CommandList* D3D11RenderThread::GetCommandList()
{
	return m_commanList;
}