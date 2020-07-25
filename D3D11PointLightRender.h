#pragma once
#include "LightRenderInF.h"
#include "D3D11Shader.h"

struct PointLightRenderParameter : LightParameter
{
	ID3D11ShaderResourceView* depthStencilDSV;
	ID3D11ShaderResourceView* colorSRV;
	ID3D11ShaderResourceView* normalSRV;
	ID3D11ShaderResourceView* specPowerSRV;
	ID3D11Buffer * GBufferUnpackCB;
	XMFLOAT3 upperLight;
	XMFLOAT3 LowerLight;
	bool transparent =false;
};

class D3D11PointLightRender :public LightRenderInF
{
public:
	D3D11PointLightRender();
	~D3D11PointLightRender();
	virtual HRESULT Initial(DXInF* pDevice) override;
	virtual void Render(void* pDeviceContext,
		LightObjInF* pObj,
		Camera* pCamera,
		LightParameter* extraParameter = NULL) override;
	virtual void Destroy() override;
private:
	D3D11Shader m_shader;
	ID3D11SamplerState* m_pSamplerState;
	ID3D11Buffer*   m_pPointLightCB;
	ID3D11Buffer*   m_pProjectionLightCB;
};

