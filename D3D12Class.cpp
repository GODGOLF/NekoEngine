#include "DX12Include.h"
#include"D3D12Class.h"
#include "FunctionHelper.h"

XMVECTORF32 g_backgroundColor = { 0.0000000f, 0.000000f, 0.000000000f, 1.000000000f };

using namespace Microsoft::WRL;

D3D12Class::D3D12Class()
{

}
D3D12Class::~D3D12Class()
{

}
HRESULT D3D12Class::OnInit(HWND hwnd, UINT width, UINT height)
{
    UINT dxgiFactoryFlags = 0;

    // Enable the debug layer (requires the Graphics Tools "optional feature").
   // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }


    //ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

   
	return S_OK;
}
void D3D12Class::BindMainRenderTarget()
{

}
void D3D12Class::EndDraw()
{

}
void D3D12Class::OnDestroy()
{

}
XMVECTORF32 D3D12Class::GetDefaultColorBg()
{
	return g_backgroundColor;
}
void D3D12Class::Reset()
{

}

void D3D12Class::EnableDebugLayer()
{
#if defined(_DEBUG)
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
    DirectXHelper::ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif
}