#ifndef _D3D11_ALPHA_GBUFFER_RENDER_THREAD_H_
#define _D3D11_ALPHA_GBUFFER_RENDER_THREAD_H_

#include "D3D11GBufferRenderThread.h"

struct AlphaGBufferInitialParameter : public GBufferInitialParameter
{

};
class D3D11AlphaGBufferRenderThread : public D3D11GBufferRenderThread
{
public:
	D3D11AlphaGBufferRenderThread();
	~D3D11AlphaGBufferRenderThread();

	virtual HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Render(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Update(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Destroy() override;

private:
	ID3D11Texture3D* m_colorLayerRT;
	ID3D11Texture3D* m_normalLayerRT;
	ID3D11Texture3D* m_specPowerLayerRT;
	ID3D11Texture3D* m_depthLayerRT;
};
#endif // !_D3D11_ALPHA_GBUFFER_RENDER_THREAD_H_

