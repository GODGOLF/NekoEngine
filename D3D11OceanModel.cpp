#include "DXInclude.h"
#include "D3D11OceanModel.h"

#define WAVE_INFO_CB_INDEX				5
#define NORMAL_TEXTURE_INDEX			0
#define ROUGHNESS_TEXTURE_INDEX			1
#define METAL_TEXTURE_INDEX				2
#define SAMPLE_INDEX					0

struct WaveInfoCB
{
	float time;
	float haveNormalTexture;
	float haveRoughnessTexture;
	float haveMetalTexture;
	D3D11WaveInfo waveInfo[3];
};
D3D11OceanModel::D3D11OceanModel() 
{
	m_pSamplerState = NULL;
	m_pConstantOceantMap = NULL;
}
D3D11OceanModel::~D3D11OceanModel()
{
	Destroy();
}
HRESULT D3D11OceanModel::Initial(char* file, ModelExtraParameter* parameter)
{
	HRESULT hr;
	hr = D3D11PlaneModel::Initial(NULL, parameter);
	if (FAILED(hr))
	{
		return hr;
	}
	D3D11OceanModelParameterInitial* d3dParameter = reinterpret_cast<D3D11OceanModelParameterInitial*>(parameter);
	ID3D11Device* d3dDevice = d3dParameter->pDevice->GetDevice();
	D3D11_BUFFER_DESC bd;
	//light constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(WaveInfoCB);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = d3dDevice->CreateBuffer(&bd, nullptr, &m_pConstantOceantMap);
	if (FAILED(hr))
		return hr;
	D3D11OceanModelParameterInitial* pParamter = (D3D11OceanModelParameterInitial*)parameter;
	hr = Texture::LoadTexture(pParamter->pDevice, pParamter->normalTexture, m_normalTexture);
	if (FAILED(hr))
		SAFE_RELEASE(m_normalTexture.texture);

	hr = Texture::LoadTexture(pParamter->pDevice, pParamter->roughnessTexture, m_roughnessTexture);
	if (FAILED(hr))
		SAFE_RELEASE(m_roughnessTexture.texture);

	hr = Texture::LoadTexture(pParamter->pDevice, pParamter->metalTexture, m_metalTexture);
	if (FAILED(hr))
		SAFE_RELEASE(m_metalTexture.texture);

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

	return S_OK;
}
void D3D11OceanModel::Render(void* pDeviceContext, ModelExtraParameter* parameter)
{
	D3D11OceanModelParameterRender* pParameter = (D3D11OceanModelParameterRender*)parameter;
	ID3D11DeviceContext* d3dDevice = (ID3D11DeviceContext*)pDeviceContext;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	d3dDevice->Map(m_pConstantOceantMap, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	WaveInfoCB* cb = (WaveInfoCB*)MappedResource.pData;
	for (int i = 0; i < WAVE_COUNT; i++)
	{
		cb->waveInfo[i].direction = pParameter->waveInfo[i].direction;
		cb->waveInfo[i].waveLength = pParameter->waveInfo[i].waveLength;
		cb->waveInfo[i].steepness = pParameter->waveInfo[i].steepness;
	}
	cb->time = pParameter->time;
	cb->haveNormalTexture = (m_normalTexture.texture != NULL) ? 1.f : 0.f;
	cb->haveRoughnessTexture = (m_roughnessTexture.texture != NULL) ? 1.f : 0.f;
	cb->haveMetalTexture = (m_metalTexture.texture != NULL) ? 1.f : 0.f;
	d3dDevice->Unmap(m_pConstantOceantMap, 0);
	d3dDevice->PSSetShaderResources(NORMAL_TEXTURE_INDEX, 1, &m_normalTexture.texture);
	d3dDevice->PSSetShaderResources(ROUGHNESS_TEXTURE_INDEX, 1, &m_roughnessTexture.texture);
	d3dDevice->PSSetShaderResources(METAL_TEXTURE_INDEX, 1, &m_metalTexture.texture);
	d3dDevice->DSSetConstantBuffers(WAVE_INFO_CB_INDEX, 1, &m_pConstantOceantMap);
	d3dDevice->PSSetConstantBuffers(WAVE_INFO_CB_INDEX, 1, &m_pConstantOceantMap);
	d3dDevice->PSSetSamplers(SAMPLE_INDEX, 1, &m_pSamplerState);
	D3D11PlaneModel::Render(pDeviceContext, parameter);
	ID3D11Buffer* nullBuffer = NULL;
	d3dDevice->DSSetConstantBuffers(WAVE_INFO_CB_INDEX, 1, &nullBuffer);
	d3dDevice->PSSetConstantBuffers(WAVE_INFO_CB_INDEX, 1, &nullBuffer);
	ID3D11ShaderResourceView* nullSRV = NULL;
	d3dDevice->PSSetShaderResources(NORMAL_TEXTURE_INDEX, 1, &nullSRV);	
	d3dDevice->PSSetShaderResources(ROUGHNESS_TEXTURE_INDEX, 1, &nullSRV);
	d3dDevice->PSSetShaderResources(METAL_TEXTURE_INDEX, 1, &nullSRV);
	ID3D11SamplerState* nullSample = NULL;
	d3dDevice->PSSetSamplers(SAMPLE_INDEX, 1, &nullSample);
}
void D3D11OceanModel::Destroy()
{
	D3D11PlaneModel::Destroy();
	SAFE_RELEASE(m_pConstantOceantMap);
	SAFE_RELEASE(m_normalTexture.texture);
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_roughnessTexture.texture);
	SAFE_RELEASE(m_metalTexture.texture);
}