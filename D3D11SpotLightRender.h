#pragma once
#include "LightRenderInF.h"
#include "D3D11Shader.h"

struct SpotLightRenderParameter : LightParameter
{
	ID3D11ShaderResourceView* depthStencilDSV = nullptr;
	ID3D11ShaderResourceView* colorSRV = nullptr;
	ID3D11ShaderResourceView* normalSRV = nullptr;
	ID3D11ShaderResourceView* specPowerSRV = nullptr;
	ID3D11Buffer * GBufferUnpackCB = nullptr;
	XMFLOAT3 upperLight = { 0.f, 0.f, 0.f };
	XMFLOAT3 LowerLight = { 0.f, 0.f, 0.f };
	bool isTransparent = false;
};

class D3D11SpotLightRender :public LightRenderInF
{
public:
	D3D11SpotLightRender();
	~D3D11SpotLightRender();
	virtual HRESULT Initial(DXInF* pDevice) override;
	virtual void Render(void* pDeviceContext,
		LightObjInF* pObj,
		Camera* pCamera,
		LightParameter* extraParameter = NULL) override;
	virtual void Destroy() override;
private:
	D3D11Shader m_shader;
	ID3D11SamplerState* m_pSamplerState;
	ID3D11Buffer*   m_pSpotLightCB;
	ID3D11Buffer*   m_pProjectionLightCB;
};


