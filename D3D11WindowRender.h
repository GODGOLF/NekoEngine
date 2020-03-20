#pragma once
#ifndef _D3D11_WINDOW_RENDER_H_
#define _D3D11_WINDOW_RENDER_H_
#include "RenderInF.h"
#include "D3D11Shader.h"

struct WindowRenderParameter : public Parameter
{
	ID3D11ShaderResourceView* texture;
};

class D3D11WindowRender : public RenderInF
{
public:
	D3D11WindowRender();

	virtual ~D3D11WindowRender();
	HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	void Render(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	void Update(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	void Destroy() override;

private:
	D3D11Shader m_shader;

	ID3D11SamplerState* m_pSamplerState;

	ID3D11Buffer * m_indexBuffer;
	ID3D11Buffer* m_vertBuffer;

};
#endif // !_WINDOW_SCREEN_H_

