#pragma once
#ifndef _D3D11_TEXTURE_RECORD_H_
#define _D3D11_TEXTURE_RECORD_H_
#include "TextureRecordInF.h"
class D3D11TextureRecord : public TextureRecordInF
{
public:
	D3D11TextureRecord();
	virtual ~D3D11TextureRecord();
	virtual HRESULT Initial(DXInF* pDevice, int width, int height) override;
	virtual void BeginRecord(void* pDeviceContext, bool clearDepth, bool clearRenderTarget) override;
	virtual void EndRecord(void* pDeviceContext) override;
	virtual void Destroy() override;
	ID3D11ShaderResourceView* GetTexture();
private:
	ID3D11RenderTargetView * m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
	ID3D11DepthStencilView* m_depthStencilView;
	D3D11_VIEWPORT m_vp;

	ID3D11RenderTargetView* m_prevRenderTarget;
	ID3D11DepthStencilView* m_prevDepthStencilView;
	D3D11_VIEWPORT m_prevVP;
};
#endif // !_D3D11_TEXTURE_RECORD_H_

