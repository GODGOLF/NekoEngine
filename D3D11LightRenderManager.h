#pragma once

#ifndef _D3D11_LIGHT_RENDER_MANAGER_H_
#define _D3D11_LIGHT_RENDER_MANAGER_H_
#include "RenderInF.h"
#include <Windows.h>
#include "LightRenderInF.h"
#include "LightObjInF.h"
#include "TextureRecordInF.h"
#include "D3D11ShadowManagerThread.h"

struct LightInitialParameter :Parameter
{
	int width;
	int height;
};
struct LightRenderParameter :Parameter
{
	Camera* pCamera = NULL;
	std::vector<LightObjInF*>* pLights = NULL;
	ID3D11ShaderResourceView* depthStencilDSV = NULL;
	ID3D11ShaderResourceView* colorSRV = NULL;
	ID3D11ShaderResourceView* normalSRV = NULL;
	ID3D11ShaderResourceView* specPowerSRV = NULL;
	D3D11ShadowManagerThread* shadowManager = NULL;
	ID3D11Buffer* voxelLightRenderCB = NULL;
	ID3D11ShaderResourceView* voxelLightPassSRV = NULL;
};


class D3D11LightRenderManager : public RenderInF
{
public:
	D3D11LightRenderManager();
	virtual ~D3D11LightRenderManager();
	virtual HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Render(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Update(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Destroy() override;

	ID3D11ShaderResourceView* GetLightSRV();
private:
	LightRenderInF * m_pDirectionLight;
	LightRenderInF * m_pAmbientLight;
	LightRenderInF * m_pPointLight;
	LightRenderInF * m_pSpotLight;

	//depth stencil state 
	ID3D11DepthStencilState* m_pDepthWriteLessStencilMaskState;
	ID3D11DepthStencilState* m_pNoDepthWriteGreatherStencilMaskState;
	ID3D11BlendState* m_pAdditiveBlendState;
	ID3D11BlendState* m_pAddBlendState;
	ID3D11RasterizerState* m_pNoDepthClipFrontRS;
	ID3D11RasterizerState* m_RSCullBack;
	ID3D11RasterizerState* m_RSCullFont;
	ID3D11SamplerState* m_clampSample;

	TextureRecordInF* m_record;
	//gBuffer
	ID3D11Buffer * m_pGBufferUnpackCB;
private:
	void PrepareForUnpack(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera);
};

#endif // !_LIGHT_RENDER_MANAGER_H_

