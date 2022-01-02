#include "DXInclude.h"
#include "D3D11TerrainModel.h"
#define HEIGHT_DIFFUSE_INDEX	0
#define HEIGHT_MAP_INDEX		1
#define SAMPLE_INDEX			0
#define HEIGHT_INFO_CB_INDEX	5

struct HeightMapStruct
{
	float texCellSpaceU;
	float texCellSpaceV;
	float worldCellSpace;
	float haveTextureDiffuse;
};

D3D11TerrainModel::D3D11TerrainModel() : m_pConstantHeightMap(NULL)
{
	m_pSamplerState = NULL;
}
D3D11TerrainModel::~D3D11TerrainModel()
{
	Destroy();
}
HRESULT D3D11TerrainModel::Initial(char* file, ModelExtraParameter* parameter)
{
	HRESULT hr;
	hr = D3D11PlaneModel::Initial(NULL, parameter);
	if (FAILED(hr))
	{
		return hr;
	}
	D3D11TerrainModelParameterInitial* d3dParameter = reinterpret_cast<D3D11TerrainModelParameterInitial*>(parameter);
	hr = Texture::LoadTexture(d3dParameter->pDevice, file,m_heightMapTex);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = Texture::LoadTexture(d3dParameter->pDevice, d3dParameter->diffuseTextureFile, m_diffuseTex);
	if (FAILED(hr))
	{
		SAFE_RELEASE(m_diffuseTex.texture);
	}
	ID3D11Device* d3dDevice = d3dParameter->pDevice->GetDevice();
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
	hr = d3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerState);
	if (FAILED(hr))
		return hr;

	D3D11_BUFFER_DESC bd;
	//light constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(HeightMapStruct);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = d3dDevice->CreateBuffer(&bd, nullptr, &m_pConstantHeightMap);
	if (FAILED(hr))
		return hr;
	return S_OK;
}
void D3D11TerrainModel::Render(void* pDeviceContext, ModelExtraParameter* parameter)
{
	ID3D11DeviceContext* d3dDevice = (ID3D11DeviceContext*)pDeviceContext;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	d3dDevice->Map(m_pConstantHeightMap, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	HeightMapStruct* cb = (HeightMapStruct*)MappedResource.pData;
	cb->texCellSpaceU = 1.f/(m_dimension * m_celSize);
	cb->texCellSpaceV = 1.f / (m_dimension * m_celSize);
	cb->worldCellSpace = (float)m_celSize;
	if (m_diffuseTex.texture)
	{
		cb->haveTextureDiffuse = 1;
	}
	else
	{
		cb->haveTextureDiffuse = 0;
	}
	d3dDevice->Unmap(m_pConstantHeightMap, 0);

	d3dDevice->DSSetShaderResources(HEIGHT_MAP_INDEX, 1, &m_heightMapTex.texture);
	d3dDevice->PSSetShaderResources(HEIGHT_MAP_INDEX, 1, &m_heightMapTex.texture);
	d3dDevice->PSSetShaderResources(HEIGHT_DIFFUSE_INDEX, 1, &m_diffuseTex.texture);
	d3dDevice->DSSetSamplers(SAMPLE_INDEX, 1, &m_pSamplerState);
	d3dDevice->PSSetSamplers(SAMPLE_INDEX, 1, &m_pSamplerState);
	d3dDevice->PSSetConstantBuffers(HEIGHT_INFO_CB_INDEX, 1, &m_pConstantHeightMap);
	D3D11PlaneModel::Render(pDeviceContext, parameter);
	ID3D11ShaderResourceView* nullSRV = NULL;
	d3dDevice->DSSetShaderResources(HEIGHT_MAP_INDEX, 1, &nullSRV);
	d3dDevice->PSSetShaderResources(HEIGHT_MAP_INDEX, 1, &nullSRV);
	d3dDevice->PSSetShaderResources(HEIGHT_DIFFUSE_INDEX, 1, &nullSRV);
	ID3D11SamplerState* nullSample = NULL;
	d3dDevice->DSSetSamplers(SAMPLE_INDEX, 1, &nullSample);
	d3dDevice->PSSetSamplers(SAMPLE_INDEX, 1, &nullSample);
	ID3D11Buffer* nullBuffer = NULL;
	d3dDevice->PSSetConstantBuffers(HEIGHT_INFO_CB_INDEX, 1, &nullBuffer);
}
void D3D11TerrainModel::Destroy()
{
	D3D11PlaneModel::Destroy();
	SAFE_RELEASE(m_heightMapTex.texture);
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pConstantHeightMap);
	SAFE_RELEASE(m_diffuseTex.texture);
}