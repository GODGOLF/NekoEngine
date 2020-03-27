#include "DXInclude.h"
#include "D3D11ImGuiRender.h"
#include "D3D11Class.h"
#include <dinput.h>
#define WHEEL_DELTA_TIME 0.01f

#define SHADER_FILE	"Data/Shader/GUIEditor.fx"

struct VERTEX_CONSTANT_BUFFER
{
	float   mvp[4][4];
};
D3D11ImGUIRender::D3D11ImGUIRender() : m_pVertexConstantBuffer(NULL),
	m_pRasterizerState(NULL),
	m_pBlendState(NULL),
	m_pDepthStencilState(NULL),
	m_pFontSampler(NULL),
	m_pVB(NULL),
	m_pIB(NULL)
{

}
D3D11ImGUIRender::~D3D11ImGUIRender()
{
	Destroy();
}

HRESULT D3D11ImGUIRender::Initial(DXInF* d3d, HWND hwnd, int width, int height)
{
	ID3D11Device* d3dDevice = ((D3D11Class*)d3d)->GetDevice();
	m_VertexBufferSize = 5000;
	m_IndexBufferSize = 10000;
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	io.BackendRendererName = "GUI_EDITOR";
	io.DisplaySize.x = (float)width;
	io.DisplaySize.y = (float)height;

	io.ImeWindowHandle = hwnd;

	//key
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Space] = VK_SPACE;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = DIK_A;
	io.KeyMap[ImGuiKey_C] = DIK_C;
	io.KeyMap[ImGuiKey_V] = DIK_V;
	io.KeyMap[ImGuiKey_X] = DIK_X;
	io.KeyMap[ImGuiKey_Y] = DIK_Y;
	io.KeyMap[ImGuiKey_Z] = DIK_Z;

	HRESULT hr = S_FALSE;
	D3D11ShaderLayout shaderLayout;
	shaderLayout.layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = m_shader.Initial(d3d, (char*)SHADER_FILE, &shaderLayout, SHADER_MODE::VS_PS_MODE);
	if (FAILED(hr)) {
		return hr;
	}
	// Create the constant buffer
	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		hr = d3dDevice->CreateBuffer(&desc, NULL, &m_pVertexConstantBuffer);
		if (FAILED(hr)) {
			return hr;
		}
	}
	// Create the blending setup
	{
		D3D11_BLEND_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AlphaToCoverageEnable = false;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = d3dDevice->CreateBlendState(&desc, &m_pBlendState);
		if (FAILED(hr)) {
			return hr;
		}
	}

	// Create the rasterizer state
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.ScissorEnable = true;
		desc.DepthClipEnable = true;
		hr = d3dDevice->CreateRasterizerState(&desc, &m_pRasterizerState);
		if (FAILED(hr)) {
			return hr;
		}
	}
	// Create depth-stencil State
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace = desc.FrontFace;
		hr = d3dDevice->CreateDepthStencilState(&desc, &m_pDepthStencilState);
		if (FAILED(hr)) {
			return hr;
		}
	}
	hr = CreateFontTexture(d3dDevice);
	if (FAILED(hr)) {
		return hr;
	}
	return S_OK;
}

void D3D11ImGUIRender::Destroy()
{
	m_shader.Destroy();
	SAFE_RELEASE(m_pVertexConstantBuffer);
	SAFE_RELEASE(m_FontTextureView);
	SAFE_RELEASE(m_pRasterizerState);
	SAFE_RELEASE(m_pBlendState);
	SAFE_RELEASE(m_pDepthStencilState);
	SAFE_RELEASE(m_pFontSampler);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);

	for (unsigned int i = 0; i < m_windows.size(); i++)
	{
		m_windows[i] = NULL;
	}
	ImGui::DestroyContext();
}
HRESULT D3D11ImGUIRender::CreateFontTexture(ID3D11Device* d3d)
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	// Upload texture to graphics system
	HRESULT hr = S_FALSE;
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D *pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = pixels;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		hr = d3d->CreateTexture2D(&desc, &subResource, &pTexture);
		if (FAILED(hr)) 
		{
			return hr;
		}
		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		hr = d3d->CreateShaderResourceView(pTexture, &srvDesc, &m_FontTextureView);
		pTexture->Release();
		if (FAILED(hr)) 
		{
			return hr;
		}
	}
	// Store our identifier
	io.Fonts->TexID = (ImTextureID)m_FontTextureView;

	// Create texture sampler
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias = 0.f;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MinLOD = 0.f;
		desc.MaxLOD = 0.f;
		hr = d3d->CreateSamplerState(&desc, &m_pFontSampler);
		if (FAILED(hr)) 
		{
			return hr;
		}
	}
	return hr;
}

void D3D11ImGUIRender::Render(DXInF* d3d, Camera* camera)
{
	ID3D11DeviceContext* d3dDeviceContext = ((D3D11Class*)d3d)->GetDeviceContext();
	ID3D11Device* d3dDevice = ((D3D11Class*)d3d)->GetDevice();
	ImGui::NewFrame();
	for (unsigned int i = 0; i < m_windows.size(); i++)
	{
		m_windows[i]->Render(camera);
	}

	// Rendering
	ImGui::Render();
	ImGui::EndFrame();
	ImDrawData* draw_data = ImGui::GetDrawData();
	// Create and grow vertex/index buffers if needed
	if (!m_pVB || m_VertexBufferSize < draw_data->TotalVtxCount)
	{
		if (m_pVB) { m_pVB->Release(); m_pVB = NULL; }
		m_VertexBufferSize = draw_data->TotalVtxCount + 5000;
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = m_VertexBufferSize * sizeof(ImDrawVert);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		if (d3dDevice->CreateBuffer(&desc, NULL, &m_pVB) < 0)
			return;
	}
	if (!m_pIB || m_IndexBufferSize < draw_data->TotalIdxCount)
	{
		if (m_pIB) { m_pIB->Release(); m_pIB = NULL; }
		m_IndexBufferSize = draw_data->TotalIdxCount + 10000;
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = m_IndexBufferSize * sizeof(ImDrawIdx);
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (d3dDevice->CreateBuffer(&desc, NULL, &m_pIB) < 0)
			return;
	}
	// Upload vertex/index data into a single contiguous GPU buffer
	D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
	if (d3dDeviceContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
		return;
	if (d3dDeviceContext->Map(m_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
		return;
	ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
	ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}
	d3dDeviceContext->Unmap(m_pVB, 0);
	d3dDeviceContext->Unmap(m_pIB, 0);
	// Setup orthographic projection matrix into our constant buffer
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
	{
		D3D11_MAPPED_SUBRESOURCE mapped_resource;
		if (d3dDeviceContext->Map(m_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
			return;
		VERTEX_CONSTANT_BUFFER* constant_buffer = (VERTEX_CONSTANT_BUFFER*)mapped_resource.pData;
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		float mvp[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
		{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
		{ 0.0f,         0.0f,           0.5f,       0.0f },
		{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
		};
		memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
		d3dDeviceContext->Unmap(m_pVertexConstantBuffer, 0);
	}
	// Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
	struct BACKUP_DX11_STATE
	{
		UINT                        ScissorRectsCount, ViewportsCount;
		D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		ID3D11RasterizerState*      RS;
		ID3D11BlendState*           BlendState;
		FLOAT                       BlendFactor[4];
		UINT                        SampleMask;
		UINT                        StencilRef;
		ID3D11DepthStencilState*    DepthStencilState;
		ID3D11ShaderResourceView*   PSShaderResource;
		ID3D11SamplerState*         PSSampler;
		ID3D11PixelShader*          PS;
		ID3D11VertexShader*         VS;
		UINT                        PSInstancesCount, VSInstancesCount;
		ID3D11ClassInstance*        PSInstances[256], *VSInstances[256];   // 256 is max according to PSSetShader documentation
		D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
		ID3D11Buffer*               IndexBuffer, *VertexBuffer, *VSConstantBuffer;
		UINT                        IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
		DXGI_FORMAT                 IndexBufferFormat;
		ID3D11InputLayout*          InputLayout;
	};
	BACKUP_DX11_STATE old;
	old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	d3dDeviceContext->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
	d3dDeviceContext->RSGetViewports(&old.ViewportsCount, old.Viewports);
	d3dDeviceContext->RSGetState(&old.RS);
	d3dDeviceContext->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
	d3dDeviceContext->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
	d3dDeviceContext->PSGetShaderResources(0, 1, &old.PSShaderResource);
	d3dDeviceContext->PSGetSamplers(0, 1, &old.PSSampler);
	old.PSInstancesCount = old.VSInstancesCount = 256;
	d3dDeviceContext->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
	d3dDeviceContext->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
	d3dDeviceContext->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
	d3dDeviceContext->IAGetPrimitiveTopology(&old.PrimitiveTopology);
	d3dDeviceContext->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
	d3dDeviceContext->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
	d3dDeviceContext->IAGetInputLayout(&old.InputLayout);

	m_shader.PreRender(d3d);
	// Setup desired DX state
	SetRenderState(draw_data, d3dDeviceContext);

	// Render command lists
	int vtx_offset = 0;
	int idx_offset = 0;
	ImVec2 clip_off = draw_data->DisplayPos;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != NULL)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					SetRenderState(draw_data, d3dDeviceContext);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Apply scissor/clipping rectangle
				const D3D11_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
				d3dDeviceContext->RSSetScissorRects(1, &r);

				// Bind texture, Draw
				ID3D11ShaderResourceView* texture_srv = (ID3D11ShaderResourceView*)pcmd->TextureId;
				d3dDeviceContext->PSSetShaderResources(0, 1, &texture_srv);
				d3dDeviceContext->DrawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}
	m_shader.PostRender(d3dDeviceContext);
	// Restore modified DX state
	d3dDeviceContext->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
	d3dDeviceContext->RSSetViewports(old.ViewportsCount, old.Viewports);
	d3dDeviceContext->RSSetState(old.RS); if (old.RS) old.RS->Release();
	d3dDeviceContext->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) old.BlendState->Release();
	d3dDeviceContext->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef); if (old.DepthStencilState) old.DepthStencilState->Release();
	d3dDeviceContext->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) old.PSShaderResource->Release();
	d3dDeviceContext->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) old.PSSampler->Release();
	d3dDeviceContext->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) old.PS->Release();
	for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
	d3dDeviceContext->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) old.VS->Release();
	d3dDeviceContext->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) old.VSConstantBuffer->Release();
	for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
	d3dDeviceContext->IASetPrimitiveTopology(old.PrimitiveTopology);
	d3dDeviceContext->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) old.IndexBuffer->Release();
	d3dDeviceContext->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) old.VertexBuffer->Release();
	d3dDeviceContext->IASetInputLayout(old.InputLayout); if (old.InputLayout) old.InputLayout->Release();
}
void D3D11ImGUIRender::SetRenderState(ImDrawData* data, ID3D11DeviceContext* d3d)
{
	// Setup viewport
	D3D11_VIEWPORT vp;
	memset(&vp, 0, sizeof(D3D11_VIEWPORT));
	vp.Width = data->DisplaySize.x;
	vp.Height = data->DisplaySize.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = vp.TopLeftY = 0;
	d3d->RSSetViewports(1, &vp);

	// Setup shader and vertex buffers
	unsigned int stride = sizeof(ImDrawVert);
	unsigned int offset = 0;
	d3d->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);
	d3d->IASetIndexBuffer(m_pIB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
	d3d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d->VSSetConstantBuffers(0, 1, &m_pVertexConstantBuffer);
	d3d->PSSetSamplers(0, 1, &m_pFontSampler);
	// Setup blend state
	const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
	d3d->OMSetBlendState(m_pBlendState, blend_factor, 0xffffffff);
	d3d->OMSetDepthStencilState(m_pDepthStencilState, 0);
	d3d->RSSetState(m_pRasterizerState);
}
void D3D11ImGUIRender::AddWindow(EditorInf* window) 
{
	m_windows.push_back(window);
}
void D3D11ImGUIRender::Update(bool* mouseClick, int mouseWheel, DirectX::XMFLOAT2 mousePos)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[0] = mouseClick[0];
	io.MouseDown[1] = mouseClick[1];
	io.MouseDown[2] = mouseClick[2];
	io.MousePos = ImVec2((float)mousePos.x, (float)mousePos.y);
	io.MouseWheel += mouseWheel * WHEEL_DELTA_TIME;
}