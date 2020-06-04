#ifndef _D3D11_DIRECTIONAL_LIGHT_RENDER_H_
#define _D3D11_DIRECTIONAL_LIGHT_RENDER_H_
#include "LightRenderInF.h"
#include "D3D11Shader.h"
#include "D3D11ShadowManagerThread.h"

struct DirectionalLightRenderParameter : LightParameter
{
	ID3D11ShaderResourceView* depthStencilDSV = NULL;
	ID3D11ShaderResourceView* colorSRV = NULL;
	ID3D11ShaderResourceView* normalSRV = NULL;
	ID3D11ShaderResourceView* specPowerSRV = NULL;
	ID3D11Buffer * GBufferUnpackCB = NULL;
	DirectionalLightSahdow* shadow = NULL;
};

class D3D11DirectionalLightRender :public LightRenderInF
{
public:
	D3D11DirectionalLightRender();
	virtual~D3D11DirectionalLightRender();
	virtual HRESULT Initial(DXInF* pDevice) override;
	virtual void Render(void* pDeviceContext,
		LightObjInF* pObj, 
		Camera* pCamera,
		LightParameter* extraParameter = NULL) override;
	virtual void Destroy() override;
private:
	D3D11Shader m_shader;
	ID3D11SamplerState* m_pSamplerState;
	ID3D11SamplerState* m_PCFSamplerState;
	ID3D11SamplerState* m_BlockerSampler;
	// Directional light constant buffer
	ID3D11Buffer*   m_pDirLightCB;
	ID3D11Buffer * m_indexBuffer;
	ID3D11Buffer* m_vertBuffer;
private:
	void UpdateDirCB(void* pDeviceContext,LightObjInF* pObj, DirectionalLightRenderParameter* extraParameter);
};

#endif // !_DIRECTION_LIGHT_RENDER_H_

