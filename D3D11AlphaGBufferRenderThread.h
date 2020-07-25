#ifndef _D3D11_ALPHA_GBUFFER_RENDER_THREAD_H_
#define _D3D11_ALPHA_GBUFFER_RENDER_THREAD_H_

#include "D3D11RenderThread.h"
#include "D3D11GBufferRenderThread.h"
#include "D3D11Shader.h"
#define TRANSPARENT_LAYER_COUNT	18
struct AlphaGBufferInitialParameter : public GBufferInitialParameter
{
	
};
struct AlphaGBufferRenderParameter : public Parameter
{
	ID3D11ShaderResourceView* nonTranparentDepthSRV;
};
class D3D11AlphaGBufferRenderThread : public D3D11GBufferRenderThread
{
public:
	D3D11AlphaGBufferRenderThread();
	virtual ~D3D11AlphaGBufferRenderThread();

	virtual HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Render(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Update(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Destroy() override;

	void SetAlphaGBufferRenderParameter(AlphaGBufferRenderParameter &parameter);

	virtual ID3D11ShaderResourceView* GetDepthView() { return m_depthLayerSRV; }
	virtual ID3D11ShaderResourceView* GetColorView() { return m_colorLayerSRV; }
	virtual ID3D11ShaderResourceView* GetNormalView() { return m_normalLayerSRV; }
	virtual ID3D11ShaderResourceView* GetSpecPowerView() { return m_specPowerLayerSRV; }

private:
	ID3D11Texture3D* m_colorLayerRT;
	ID3D11Texture3D* m_normalLayerRT;
	ID3D11Texture3D* m_specPowerLayerRT;
	ID3D11Texture3D* m_depthLayerRT;

	// GBuffer shader resource views
	ID3D11ShaderResourceView* m_colorLayerSRV;
	ID3D11ShaderResourceView* m_normalLayerSRV;
	ID3D11ShaderResourceView* m_specPowerLayerSRV;
	ID3D11ShaderResourceView* m_depthLayerSRV;
	//voxel texture UAV
	ID3D11UnorderedAccessView* m_colorLayerUAV;
	ID3D11UnorderedAccessView* m_normalLayerUAV;
	ID3D11UnorderedAccessView* m_specPowerLayerUAV;
	ID3D11UnorderedAccessView* m_depthLayerUAV;

	// GBuffer render views
	ID3D11RenderTargetView* m_colorLayerRTV;
	ID3D11RenderTargetView* m_normalLayerRTV;
	ID3D11RenderTargetView* m_specPowerLayerRTV;
	ID3D11RenderTargetView* m_depthLayerRTV;

	ID3D11Buffer* m_resCB;

private:
	virtual void RenderObj() override;

	virtual void ThreadExcecute() override;

	void UpdateResCB();

	AlphaGBufferRenderParameter m_alphaRenderParameter;

	int m_indexRecord;
};
#endif // !_D3D11_ALPHA_GBUFFER_RENDER_THREAD_H_

