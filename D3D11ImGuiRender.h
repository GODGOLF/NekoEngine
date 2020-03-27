#pragma once
/*
GUIEditorRender.h

Author : Zhuang Theerapong

*/
#pragma once

#ifndef _D3D11_IMGUI_RENDER_H_
#define _D3D11_IMGUI_RENDER_H_
#include "D3D11Shader.h"
#include "imGUI\imgui.h"
#include "EditorInf.h"
#include "ImGuiIRenderInF.h"
class D3D11ImGUIRender :public ImGuiRenderInF{
public:
	D3D11ImGUIRender();
	virtual ~D3D11ImGUIRender();
	void AddWindow(EditorInf* window) override;
	void Update(bool* mouseClick, int mouseWheel, DirectX::XMFLOAT2 mousePos);
	void Destroy() override;
private:
	HRESULT Initial(DXInF* d3d, HWND hwnd, int width, int height) override;
	void Render(DXInF* d3d, Camera* camera) override;
private:
	ID3D11Buffer * m_pVertexConstantBuffer;
	ID3D11ShaderResourceView*	m_FontTextureView;
	ID3D11RasterizerState*		m_pRasterizerState;
	ID3D11BlendState*			m_pBlendState;
	ID3D11DepthStencilState*	m_pDepthStencilState;
	ID3D11SamplerState*			m_pFontSampler;
	ID3D11Buffer*				m_pVB;
	ID3D11Buffer*				m_pIB;
	int m_VertexBufferSize, m_IndexBufferSize;
	//shader
	D3D11Shader m_shader;
	//method
	HRESULT CreateFontTexture(ID3D11Device* d3d);
	void SetRenderState(ImDrawData* data, ID3D11DeviceContext* d3d);
	std::vector<EditorInf*> m_windows;
};
#endif // !_GUI_EDITOR_H_
