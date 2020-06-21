#include "DXInclude.h"
#include "D3D11SkyBoxModel.h"
#include "ModelDataStructure.h"
#define DIFFUSE_TEXTURE_INDEX		3
#define MATERIAL_CB_INDEX			3

#define DISPLACEMENT_CB_INDEX		5
#define DISPLACEMENT_TEX_INDEX		2
#define SHADER_TYPE_ID				1
#define TEXTURE_SAMPLE				0

struct MaterialConstant
{
	MaterialConstant();
	XMFLOAT4 diffuseColor;
	XMFLOAT4 specularColor;
	XMFLOAT3 haveTexture;
	float specExp;
	float  metallic;
	float roughness;
	float shaderTypeID;
	float pad;
};
struct DisPlacementMapCB
{
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMaxTessFactor;
	float gMinTessFactor;
	XMFLOAT3 gCameraPos;
	float gHeightScale;
	float haveDisplacementMap;
	float pad[3];
};

D3D11SkyBoxModel::D3D11SkyBoxModel() : 
	m_pConstantMaterial(NULL), 
	m_pConstantTessa(NULL), 
	m_pSamplerState(NULL),
	m_pVertexBuffer(NULL),
	m_pIndexBuffer(NULL),
	m_cullFrontRS(NULL)
{

}
D3D11SkyBoxModel::~D3D11SkyBoxModel()
{
	Destroy();
}
HRESULT D3D11SkyBoxModel::Initial(char* file, ModelExtraParameter* parameter)
{
	D3D11SkyBoxModelParameterInitial* pParameter = (D3D11SkyBoxModelParameterInitial*)parameter;
	float size = pParameter->size;
	m_diffuseColor = pParameter->diffuseColor;
	VertexAnime v[] =
	{
		// Front Face
		{
			
			XMFLOAT3(-1.0f* size, -1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4 (-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{
			XMFLOAT3(-1.0f* size,  1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{
			XMFLOAT3(1.0f* size,  1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{
			XMFLOAT3(1.0f* size, -1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		// Back Face
		{
			
			XMFLOAT3(-1.0f* size, -1.0f* size, 1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(1.0f* size, -1.0f* size, 1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		
		{

			XMFLOAT3(1.0f* size,  1.0f* size, 1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		
		{

			XMFLOAT3(-1.0f* size,  1.0f* size, 1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		// Top Face
		{

			XMFLOAT3(-1.0f* size, 1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(-1.0f* size, 1.0f* size,  1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(1.0f* size, 1.0f* size,  1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(1.0f* size, 1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		// Bottom Face
		{

			XMFLOAT3(-1.0f* size, -1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(1.0f* size, -1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(1.0f* size, -1.0f* size,  1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(-1.0f* size, -1.0f* size,  1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		// Left Face
		{

			XMFLOAT3(-1.0f* size, -1.0f* size,  1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(-1.0f* size,  1.0f* size,  1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(-1.0f* size,  1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(-1.0f* size, -1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		// Right Face
		{

			XMFLOAT3(1.0f* size, -1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(1.0f* size,  1.0f* size, -1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(1.0f* size,  1.0f* size,  1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
		{

			XMFLOAT3(1.0f* size, -1.0f* size,  1.0f* size),
			XMFLOAT3(0.f,0.f,0.f),
			XMFLOAT2(0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMFLOAT4(0.f,0.f,0.f,0.f),
			XMUINT4(-1,-1,-1,-1),
			XMFLOAT4(0.0f,0.0f,0.0f,0.0f)
		},
	};
	WORD indices[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  5,  6,
		4,  6,  7,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};
	ID3D11Device* device = pParameter->pDevice->GetDevice();
	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	//vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VertexAnime) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &v[0];
	hr = device->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr)) {
		return hr;
	}

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &indices[0];
	hr = device->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr)) {
		return hr;
	}
	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampDesc, &m_pSamplerState);
	if (FAILED(hr)) {
		return hr;
	}

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(DisPlacementMapCB);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = device->CreateBuffer(&bd, nullptr, &m_pConstantTessa);
	if (FAILED(hr))
		return hr;

	//light constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MaterialConstant);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bd, nullptr, &m_pConstantMaterial);
	if (FAILED(hr))
		return hr;
	hr = Texture::GenTextureCubeMap(pParameter->pDevice,pParameter->textureFile,m_diffuseTex);
	if (FAILED(hr))
		return hr;

	D3D11_RASTERIZER_DESC descRast = {
		D3D11_FILL_SOLID,
		D3D11_CULL_FRONT,
		FALSE,
		D3D11_DEFAULT_DEPTH_BIAS,
		D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		FALSE
	};
	descRast.DepthBias = 90;
	descRast.SlopeScaledDepthBias = 2.f;
	descRast.DepthBiasClamp = 0.0f;
	descRast.DepthClipEnable = TRUE;
	hr = device->CreateRasterizerState(&descRast, &m_cullFrontRS);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void D3D11SkyBoxModel::Render(void* pDeviceContext, ModelExtraParameter* parameter)
{
	D3D11SkyBoxModelParameterRender* d3dParameter = (D3D11SkyBoxModelParameterRender*)parameter;
	

	ID3D11DeviceContext* deviceContext = (ID3D11DeviceContext*)pDeviceContext;

	ID3D11RasterizerState* prevState = NULL;
	deviceContext->RSGetState(&prevState);
	deviceContext->RSSetState(m_cullFrontRS);

	UpdateTesseration(deviceContext, d3dParameter);
	//bind MVP buffer
	d3dParameter->pMVP->BindConstantMVP(pDeviceContext, d3dParameter->pCamera,
		XMMatrixIdentity(),
		d3dParameter->pModelInfo->position,
		d3dParameter->pModelInfo->GetQuaternion(),
		d3dParameter->pModelInfo->scale);

	MaterialConstant material = MaterialConstant();
	material.diffuseColor = m_diffuseColor;
	material.specularColor = XMFLOAT4(1, 1, 1, 1);
	material.specExp = 0;
	material.roughness = d3dParameter->pModelInfo->roughness;
	material.metallic = d3dParameter->pModelInfo->metallic;
	material.shaderTypeID = SHADER_TYPE_ID;
	material.haveTexture = XMFLOAT3(2,0,0);

	deviceContext->UpdateSubresource(m_pConstantMaterial, 0, nullptr, &material, 0, 0);
	UINT stride = sizeof(VertexAnime);
	UINT offset = 0;

	deviceContext->VSSetConstantBuffers(DISPLACEMENT_CB_INDEX, 1, &m_pConstantTessa);
	deviceContext->DSSetConstantBuffers(DISPLACEMENT_CB_INDEX, 1, &m_pConstantTessa);
	deviceContext->PSSetShaderResources(DIFFUSE_TEXTURE_INDEX, 1, &m_diffuseTex.texture);

	deviceContext->PSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &m_pConstantMaterial);
	deviceContext->PSSetSamplers(TEXTURE_SAMPLE, 1, &m_pSamplerState);
	deviceContext->DSSetSamplers(TEXTURE_SAMPLE, 1, &m_pSamplerState);
	deviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetPrimitiveTopology(d3dParameter->drawType);
	deviceContext->DrawIndexed(36, 0, 0);

	ID3D11ShaderResourceView* texture = NULL;
	deviceContext->PSSetShaderResources(DIFFUSE_TEXTURE_INDEX, 1, &texture);
	//unbind sampler
	ID3D11SamplerState* nullSampler = NULL;
	deviceContext->PSSetSamplers(TEXTURE_SAMPLE, 1, &nullSampler);
	deviceContext->DSSetSamplers(TEXTURE_SAMPLE, 1, &nullSampler);
	d3dParameter->pMVP->UnbindConstantMVP(deviceContext);

	deviceContext->RSSetState(prevState);
	if (prevState)
	{
		prevState->Release();
	}
	ID3D11Buffer* nullBuffer = NULL;
	deviceContext->PSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &nullBuffer);
}
void D3D11SkyBoxModel::Destroy()
{
	SAFE_RELEASE(m_pConstantMaterial);
	SAFE_RELEASE(m_pConstantTessa);
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_diffuseTex.texture);
	SAFE_RELEASE(m_cullFrontRS);
}
void D3D11SkyBoxModel::UpdateTesseration(ID3D11DeviceContext* pDeviceContext, D3D11SkyBoxModelParameterRender* pParameter)
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pDeviceContext->Map(m_pConstantTessa, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	DisPlacementMapCB* cb = (DisPlacementMapCB*)MappedResource.pData;
	cb->gHeightScale = pParameter->pModelInfo->heightScale;
	cb->gMaxTessDistance = pParameter->pModelInfo->maxTessDistance;
	cb->gMinTessDistance = pParameter->pModelInfo->minTessDistance;
	cb->gMinTessFactor = pParameter->pModelInfo->minTessFactor;
	cb->gMaxTessFactor = pParameter->pModelInfo->maxTessFactor;
	cb->haveDisplacementMap = 0;
	XMStoreFloat3(&cb->gCameraPos, pParameter->pCamera->GetPosition());
	pDeviceContext->Unmap(m_pConstantTessa, 0);
}