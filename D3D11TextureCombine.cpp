#include "DXInclude.h"
#include "D3D11TextureCombine.h"
#include"D3D11Class.h"

#define SHADER_FILE	"Data/Shader/TextureCombineByDepth.fx"

#define TEXTURE_01_INDEX		0
#define TEXTURE_02_INDEX		1
#define DEPTH_TEXTURE_01_INDEX	2
#define DEPTH_TEXTURE_02_INDEX	3
#define SAMPLE_INDEX			0
struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 tex;
};
D3D11TextureCombine::D3D11TextureCombine()
	:m_indexBuffer(NULL),m_pSamplerState(NULL),m_vertBuffer(NULL)
{

}
D3D11TextureCombine::~D3D11TextureCombine()
{
	Destroy();
}
HRESULT D3D11TextureCombine::Initial(DXInF* pD3Device, int width, int height)
{
	HRESULT hr = S_FALSE;
	hr = m_record.Initial(pD3Device, width, height);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11ShaderLayout shaderLayout;
	shaderLayout.layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = m_shader.Initial(pD3Device, (char*)SHADER_FILE,&shaderLayout, SHADER_MODE::VS_PS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}
	ID3D11Device* device = ((D3D11Class*)pD3Device)->GetDevice();
	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 0;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampDesc, &m_pSamplerState);
	if (FAILED(hr))
	{
		return hr;
	}
	//Create the vertex buffer
	Vertex v[] =
	{
		// Front Face
		 XMFLOAT3(-1.0f, -1.0f, 0.0f),XMFLOAT2(0.f, 1.0f),
		XMFLOAT3(-1.0f,  1.0f, 0.0f),XMFLOAT2(0.0f, 0.0f),
		XMFLOAT3(1.0f,  1.0f, 0.0f),XMFLOAT2(1.0f, 0.0f),
		XMFLOAT3(1.0f, -1.0f, 0.0f),XMFLOAT2(1.0f, 1.0f)
	};

	WORD indices[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,
	};
	D3D11_BUFFER_DESC bd;
	//vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &v[0];
	hr = device->CreateBuffer(&bd, &InitData, &m_vertBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &indices[0];
	hr = device->CreateBuffer(&bd, &InitData, &m_indexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void D3D11TextureCombine::Render(void* pDeviceContext,
	void* textureInput1,
	void*textureInput2,
	void* depthInput1,
	void* depthInput2)
{
	m_record.BeginRecord(pDeviceContext, true, true);
	m_shader.PreRender(pDeviceContext);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11DeviceContext* deviceContext = (ID3D11DeviceContext*)pDeviceContext;
	ID3D11ShaderResourceView* tex1 = (ID3D11ShaderResourceView*)textureInput1;
	deviceContext->PSSetShaderResources(TEXTURE_01_INDEX, 1, &tex1);
	ID3D11ShaderResourceView* tex2 = (ID3D11ShaderResourceView*)textureInput2;
	deviceContext->PSSetShaderResources(TEXTURE_02_INDEX, 1, &tex2);
	ID3D11ShaderResourceView* depthTex1 = (ID3D11ShaderResourceView*)depthInput1;
	deviceContext->PSSetShaderResources(DEPTH_TEXTURE_01_INDEX, 1, &depthTex1);
	ID3D11ShaderResourceView* depthTex2 = (ID3D11ShaderResourceView*)depthInput2;
	deviceContext->PSSetShaderResources(DEPTH_TEXTURE_02_INDEX, 1, &depthTex2);
	deviceContext->IASetVertexBuffers(0, 1, &m_vertBuffer, &stride, &offset);
	// Set index buffer
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->PSSetSamplers(SAMPLE_INDEX, 1, &m_pSamplerState);
	// Set primitive topology
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Render a triangle
	deviceContext->DrawIndexed(6, 0, 0);
	m_shader.PostRender(pDeviceContext);
	ID3D11ShaderResourceView* nullSRV = NULL;
	deviceContext->PSSetShaderResources(TEXTURE_01_INDEX, 1, &nullSRV);
	deviceContext->PSSetShaderResources(TEXTURE_02_INDEX, 1, &nullSRV);
	deviceContext->PSSetShaderResources(DEPTH_TEXTURE_01_INDEX, 1, &nullSRV);
	deviceContext->PSSetShaderResources(DEPTH_TEXTURE_02_INDEX, 1, &nullSRV);
	m_record.EndRecord(pDeviceContext);
}
void D3D11TextureCombine::Destroy()
{
	m_record.Destroy();
	m_shader.Destroy();
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_vertBuffer);
}
void* D3D11TextureCombine::GetOutput()
{
	return m_record.GetTexture();
}