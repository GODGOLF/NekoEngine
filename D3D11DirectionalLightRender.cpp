#include "DXInclude.h"
#include "D3D11DirectionalLightRender.h"
#include "D3D11Class.h"
#define SHADER_FILE "Data/Shader/DirectionalLight.fx"
#define DEPTH_TEXTURE	0
#define COLOR_SPEC_TEXTURE	1
#define NORMAL_TEXTURE	2
#define SPEC_POWER_TEXTURE	3
#define PREPARE_FOR_UNPACK_INDEX	1
#define DIR_CB_INDEX	2
#define SAMPLE_INDEX	0

struct CB_DIRECTIONAL
{
	XMFLOAT3 vDirToLight;
	float pad0;
	XMFLOAT4 vDirectionalColor;
	float intensity;
	float pad2[7];
};
struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 tex;
};
D3D11DirectionalLightRender::D3D11DirectionalLightRender()
	:m_indexBuffer(NULL),m_pDirLightCB(NULL),m_pSamplerState(NULL),
	m_vertBuffer(NULL)
{

}
D3D11DirectionalLightRender::~D3D11DirectionalLightRender()
{
	Destroy();
}

HRESULT D3D11DirectionalLightRender::Initial(DXInF* pDevice)
{
	HRESULT hr;
	D3D11ShaderLayout shaderLayout;
	shaderLayout.layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	hr = m_shader.Initial(pDevice, (char*)SHADER_FILE, &shaderLayout, SHADER_MODE::VS_PS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}
	ID3D11Device* pd3dDevice = ((D3D11Class*)pDevice)->GetDevice();
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
	hr = pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerState);

	if (FAILED(hr))
	{
		return hr;
	}
	// Create direction light CB
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CB_DIRECTIONAL);
	hr = pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pDirLightCB);
	if (hr != S_OK) {
		return hr;
	}
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
	hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_vertBuffer);
	if (hr != S_OK) {
		return hr;
	}
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &indices[0];
	hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_indexBuffer);
	if (hr != S_OK) {
		return hr;
	}
	return S_OK;
}
void D3D11DirectionalLightRender::Render(void* pDeviceContext, LightObjInF* obj,Camera* pCamera, LightParameter* extraParameter)
{
	ID3D11DeviceContext* pd3dDeviceContext = (ID3D11DeviceContext*)pDeviceContext;
	m_shader.PreRender(pDeviceContext);
	DirectionalLightRenderParameter* parameter = (DirectionalLightRenderParameter*)extraParameter;

	pd3dDeviceContext->PSSetShaderResources(DEPTH_TEXTURE, 1, &parameter->depthStencilDSV);
	pd3dDeviceContext->PSSetShaderResources(COLOR_SPEC_TEXTURE, 1, &parameter->colorSRV);
	pd3dDeviceContext->PSSetShaderResources(NORMAL_TEXTURE, 1, &parameter->normalSRV);
	pd3dDeviceContext->PSSetShaderResources(SPEC_POWER_TEXTURE, 1, &parameter->specPowerSRV);
	
	//update light info into buffer
	UpdateDirCB(pd3dDeviceContext, obj);
	pd3dDeviceContext->PSSetConstantBuffers(DIR_CB_INDEX, 1, &m_pDirLightCB);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_vertBuffer, &stride, &offset);
	// Set index buffer
	pd3dDeviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	//set sample
	pd3dDeviceContext->PSSetSamplers(SAMPLE_INDEX, 1, &m_pSamplerState);

	pd3dDeviceContext->PSSetConstantBuffers(PREPARE_FOR_UNPACK_INDEX, 1, &parameter->GBufferUnpackCB);

	// Set primitive topology
	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Render a triangle
	pd3dDeviceContext->DrawIndexed(6, 0, 0);
	//clear buffer
	ID3D11Buffer* nullBuffer = NULL;
	pd3dDeviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	pd3dDeviceContext->IASetIndexBuffer(nullBuffer, DXGI_FORMAT_R16_UINT, 0);
	pd3dDeviceContext->PSSetConstantBuffers(PREPARE_FOR_UNPACK_INDEX, 1, &nullBuffer);
	pd3dDeviceContext->PSSetConstantBuffers(DIR_CB_INDEX, 1, &nullBuffer);
	//clear texture and package
	ID3D11ShaderResourceView* textureNULL = NULL;
	pd3dDeviceContext->PSSetShaderResources(DEPTH_TEXTURE, 1, &textureNULL);
	pd3dDeviceContext->PSSetShaderResources(COLOR_SPEC_TEXTURE, 1, &textureNULL);
	pd3dDeviceContext->PSSetShaderResources(NORMAL_TEXTURE, 1, &textureNULL);
	pd3dDeviceContext->PSSetShaderResources(SPEC_POWER_TEXTURE, 1, &textureNULL);
	ID3D11SamplerState* nullSamplerState = NULL;
	pd3dDeviceContext->PSSetSamplers(SAMPLE_INDEX, 1, &nullSamplerState);

	m_shader.PostRender(pDeviceContext);
}
void D3D11DirectionalLightRender::Destroy()
{
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pDirLightCB);
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertBuffer);
	m_shader.Destroy();
}
void D3D11DirectionalLightRender::UpdateDirCB(void* pDeviceContext, LightObjInF* pObj)
{
	ID3D11DeviceContext* pd3dDeviceContext = (ID3D11DeviceContext*)pDeviceContext;
	// Fill the directional and ambient values constant buffer
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dDeviceContext->Map(m_pDirLightCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CB_DIRECTIONAL* pDirectionalValuesCB = (CB_DIRECTIONAL*)MappedResource.pData;
	DirectionLightObj* pDirObj = (DirectionLightObj*)pObj;
	XMFLOAT3 dir = pDirObj->Direction;
	dir.x = -dir.x;
	dir.y = -dir.y;
	dir.z = -dir.z;
	pDirectionalValuesCB->vDirToLight = dir;
	pDirectionalValuesCB->vDirectionalColor = pDirObj->Color;
	pDirectionalValuesCB->intensity = pDirObj->Intensity;
	pd3dDeviceContext->Unmap(m_pDirLightCB, 0);
}
