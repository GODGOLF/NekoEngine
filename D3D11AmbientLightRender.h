#pragma once
#include "LightRenderInF.h"
#include "D3D11Shader.h"

struct AmbientLightRenderParameter : LightParameter
{
	ID3D11ShaderResourceView* depthStencilDSV;
	ID3D11ShaderResourceView* colorSRV;
	ID3D11ShaderResourceView* normalSRV;
	ID3D11ShaderResourceView* specPowerSRV;
	ID3D11Buffer * GBufferUnpackCB;
	XMFLOAT3 upperLight;
	XMFLOAT3 LowerLight;
	bool tranparent = false;
};

class D3D11AmbientLightRender :public LightRenderInF
{
public:
	D3D11AmbientLightRender();
	~D3D11AmbientLightRender();
	virtual HRESULT Initial(DXInF* pDevice) override;
	virtual void Render(void* pDeviceContext,
		LightObjInF* pObj,
		Camera* pCamera,
		LightParameter* extraParameter = NULL) override;
	virtual void Destroy() override;
private:
	D3D11Shader m_shader;
	ID3D11SamplerState* m_pSamplerState;
	ID3D11Buffer*   m_pAmbientLightCB;
	ID3D11Buffer * m_indexBuffer;
	ID3D11Buffer* m_vertBuffer;
private:
	void UpdateAmbientCB(void* pDeviceContext, LightObjInF* pObj, AmbientLightRenderParameter* extraParameter);
};
