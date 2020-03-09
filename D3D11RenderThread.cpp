#include "DXInclude.h"
#include "D3D11RenderThread.h"
#include "D3D11Class.h"

D3D11RenderThread::D3D11RenderThread() 
{
}
D3D11RenderThread::~D3D11RenderThread()
{
	Destroy();
}
HRESULT D3D11RenderThread::Initial(DXInF* pDevice)
{
	HRESULT hr;
	D3D11Class* pDevice11 = (D3D11Class*)pDevice;

	hr = pDevice11->GetDevice()->CreateDeferredContext(0, &m_deviceContext);

	if (!SUCCEEDED(hr)) {
		return hr;
	}
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
	//test
	for (int i = 0; i < 10; i++)
	{
		OutputDebugString("Test11\n");
	}
	
}
ID3D11CommandList* D3D11RenderThread::GetCommandList()
{
	return m_commanList;
}