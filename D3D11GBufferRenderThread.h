#pragma once
#ifndef _D3D11_GBUFFER_RENDER_THREAD_H_
#define _D3D11_GBUFFER_RENDER_THREAD_H_
#include "D3D11RenderThread.h"
#include "ObjSceneInF.h"
#include "ModelInF.h"
#include "D3DModelInF.h"
#include <map>
#include "Camera.h"
#include "D3D11MVP.h"
#include "FrustumCullling.h"
#include "D3D11GBufferShaderManager.h"

struct GBufferInitialParameter : Parameter
{
	int width;
	int height;
};
struct GBufferRenderParameter : Parameter
{
	std::vector<ModelInF*>* m_modelDataList;
	std::map<std::string, D3DModelInF*>* m_modelObjectList;
	Camera* pCamera;
};

class D3D11GBufferRenderThread : public D3D11RenderThread
{
public:

	D3D11GBufferRenderThread();
	virtual ~D3D11GBufferRenderThread();
	virtual HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Render(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Update(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Destroy() override;

	void SetGBufferRenderParameter(ObjScene* pParameter, Camera* camera);

	ID3D11ShaderResourceView* GetDepthView() { return m_DepthStencilSRV; }
	ID3D11ShaderResourceView* GetColorView() { return m_ColorSpecIntensitySRV; }
	ID3D11ShaderResourceView* GetNormalView() { return m_NormalSRV; }
	ID3D11ShaderResourceView* GetSpecPowerView() { return m_SpecPowerSRV; }

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}

private:
	virtual void ThreadExcecute() override;

	virtual void RenderObj();

	ID3D11Buffer * m_pGBufferUnpackCB;
	ID3D11Buffer * m_pFrustumCB;
	// GBuffer textures
	ID3D11Texture2D * m_DepthStencilRT;
	ID3D11Texture2D* m_ColorSpecIntensityRT;
	ID3D11Texture2D* m_NormalRT;
	ID3D11Texture2D* m_SpecPowerRT;


	// GBuffer shader resource views
	ID3D11ShaderResourceView* m_DepthStencilSRV;
	ID3D11ShaderResourceView* m_ColorSpecIntensitySRV;
	ID3D11ShaderResourceView* m_NormalSRV;
	ID3D11ShaderResourceView* m_SpecPowerSRV;

	ID3D11DepthStencilState *m_DepthStencilState;

	D3D11_VIEWPORT m_Vp;
	// GBuffer render views
	ID3D11RenderTargetView * m_ColorSpecIntensityRTV;
	ID3D11RenderTargetView* m_NormalRTV;
	ID3D11RenderTargetView* m_SpecPowerRTV;
	//depth view
	ID3D11DepthStencilView* m_DepthStencilDSV;
	ID3D11DepthStencilView* m_DepthStencilReadOnlyDSV;

	D3D11MVP m_mvp;
	//nessary variable for main thread
	GBufferRenderParameter* m_RenderParameter;

	//cull 
	ID3D11RasterizerState* m_RSCullBack;

	FrustumCulling m_culling;

	D3D11GBufferShaderManager m_shaderManager;
protected:
	bool m_isTranparent;
};


#endif // !_D3D11_GBUFFER_RENDER_THREAD_H_




