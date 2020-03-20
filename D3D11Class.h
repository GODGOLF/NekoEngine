#ifndef D3D11_CLASS_H
#define D3D11_CLASS_H
#include "DXInF.h"
class D3D11Class : public DXInF {
public:
	D3D11Class();
	~D3D11Class();
	HRESULT OnInit(HWND hwnd, UINT width, UINT height);
	void BindMainRenderTarget();
	void EndDraw();
	void OnDestroy();
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
	ID3D11RenderTargetView* GetRenderTargetView();
	ID3D11DepthStencilView* GetDepthStencilView();
	void SetAlphaBlend(bool isTurnOn, bool alphaCover = false);
	XMVECTORF32* GetDefaultColorBg();
	void Reset();
	void SetCullRaterilization(D3D10_CULL_MODE mode);


	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
private:


	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*           g_pd3dDevice = nullptr;
	ID3D11DeviceContext*    g_pImmediateContext = nullptr;
	IDXGISwapChain*         g_pSwapChain = nullptr;

	ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
	ID3D11BlendState* m_alphaEnableBlendingState;
	ID3D11BlendState* m_alphaCoverEnableBlendingState;
	ID3D11BlendState* m_alphaDisableBlendingState;


	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11RasterizerState* m_RSCullBack;
	ID3D11RasterizerState* m_RSCullNone;
	ID3D11RasterizerState* m_RSCullFront;
	ID3D11ShaderResourceView* m_shaderResourceViewMap;
	XMVECTORF32 g_backgroundColor_ = { 0.0000000f, 0.000000f, 0.000000000f, 1.000000000f };
	D3D11_VIEWPORT m_vp;

};
#endif // !D3D11_CLASS_H