#include "DXInclude.h"
#include "D3D11Model.h"
#include "TextureLoader.h"
#define DIFFUSE_TEXTURE_INDEX 0
#define NORMAL_TEXTURE_INDEX 1
#define TEXTURE_SAMPLE	0

#define SKELETON_MATRIX_CB_INDEX	2
#define MATERIAL_CB_INDEX	3

D3D11Model::D3D11Model() : m_pConstantLighting(NULL),m_pConstantSkeleton(NULL), m_pSamplerState(NULL)
{

}
D3D11Model::~D3D11Model()
{
	Destroy();
}

HRESULT D3D11Model::Initial(char* file, ModelExtraParameter* parameter)
{
	bool result;
	D3D11ModelParameterInitial* d3dParameter = (D3D11ModelParameterInitial*)parameter;
	result = m_model.LoadFBX(d3dParameter->pFbxManager, file);
	if (!result)
	{
		return S_FALSE;
	}
	std::vector<FBXModelData>* list = m_model.GetModelList();
	ID3D11Device* device = d3dParameter->pDevice->GetDevice();

	//allocate memmory
	m_modelBuffer.resize(list->size());
	//create buffer  
	for (unsigned int i = 0; i < list->size(); i++)
	{
		HRESULT hr;
		D3D11_BUFFER_DESC bd;
		//vertex buffer
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VertexAnime) * (unsigned int)list->operator[](i).vertrics.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		D3D11_SUBRESOURCE_DATA InitData;
		//ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = &list->operator[](i).vertrics[0];
		hr = device->CreateBuffer(&bd, &InitData, &m_modelBuffer[i].g_pVertexBuffer);
		if (FAILED(hr))
			return hr;
		//index buffer
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(WORD) * list->operator[](i).index.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = &list->operator[](i).index[0];
		hr = device->CreateBuffer(&bd, &InitData, &m_modelBuffer[i].g_pIndexBuffer);
		if (FAILED(hr))
			return hr;
		for (auto &j : list->operator[](i).material)
		{
			if (m_textureSRV.count(std::string(j.name)) == 0)
			{
				if (strlen(j.mat.diffuseTexture) !=0)
				{
					Texture::LoadTexture((D3D11Class*)device, j.mat.diffuseTexture, m_textureSRV[j.name].diffuseTex);
				}
				if (strlen(j.mat.normalTexture) != 0)
				{
					Texture::LoadTexture((D3D11Class*)device, j.mat.normalTexture, m_textureSRV[j.name].normalTex);
				}
			}
		}
	}

	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	//light constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MaterialConstant);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bd, nullptr, &m_pConstantLighting);
	if (FAILED(hr))
		return hr;

	//skeleton constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantSkeleton);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bd, nullptr, &m_pConstantSkeleton);
	if (FAILED(hr))
		return hr;

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
	if (FAILED(hr))
		return hr;

	return S_OK;
}
void D3D11Model::Render(void* pd3dDeviceContext, ModelExtraParameter* parameter)
{
	std::vector<FBXModelData>* list = m_model.GetModelList();

	D3D11ModelParameterRender* d3dParameter = (D3D11ModelParameterRender*)parameter;

	ID3D11DeviceContext* pDeviceContext = (ID3D11DeviceContext*)pd3dDeviceContext;

	UINT stride = sizeof(VertexAnime);
	UINT offset = 0;
	for (unsigned int i = 0; i < list->size(); i++) {
		pDeviceContext->IASetVertexBuffers(0, 1, &m_modelBuffer[i].g_pVertexBuffer, &stride, &offset);
		// Set index buffer
		pDeviceContext->IASetIndexBuffer(m_modelBuffer[i].g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		// Set primitive topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//bind MVP buffer
		D3D11MVPParameter mvpParameter;
		mvpParameter.deviceContext = pDeviceContext;
		d3dParameter->pMVP->BindConstantMVP(&mvpParameter,d3dParameter->pCamera, 
			list->operator[](i).DefaultMatrix, 
			d3dParameter->pModelInfo->position, 
			d3dParameter->pModelInfo->rotation, 
			d3dParameter->pModelInfo->scale);

		for (auto &j : list->operator[](i).material) {
			MaterialConstant material;
			material.diffuseColor = j.mat.diffuseColor;
			material.specularColor = j.mat.specularColor;
			material.shiness = j.mat.shiness;
			XMFLOAT3 haveTex = XMFLOAT3(0, 0, 0);
			 
			if (m_textureSRV[j.name].diffuseTex.texture != NULL) {
				pDeviceContext->PSSetShaderResources(DIFFUSE_TEXTURE_INDEX, 1, &m_textureSRV[j.name].diffuseTex.texture);
				haveTex.x = 1;
			}
			if (m_textureSRV[j.name].normalTex.texture != NULL) {
				pDeviceContext->PSSetShaderResources(NORMAL_TEXTURE_INDEX, 1, &m_textureSRV[j.name].normalTex.texture);
				haveTex.y = 1;
			}
			if (m_model.m_haveAnimation) {
				pDeviceContext->VSSetConstantBuffers(SKELETON_MATRIX_CB_INDEX, 1, &m_pConstantSkeleton);
			}
			material.haveTexture = haveTex;
			pDeviceContext->UpdateSubresource(m_pConstantLighting, 0, nullptr, &material, 0, 0);
			pDeviceContext->PSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &m_pConstantLighting);
			pDeviceContext->PSSetSamplers(TEXTURE_SAMPLE, 1, &m_pSamplerState);
			//draw object

			pDeviceContext->DrawIndexed(j.count, j.startIndex,0);
			//unbind texture
			ID3D11ShaderResourceView* texture = NULL;
			pDeviceContext->PSSetShaderResources(DIFFUSE_TEXTURE_INDEX, 1, &texture);
			pDeviceContext->PSSetShaderResources(NORMAL_TEXTURE_INDEX, 1, &texture);
			//unbind buffer
			ID3D11Buffer* nullBuffer = NULL;
			pDeviceContext->VSSetConstantBuffers(SKELETON_MATRIX_CB_INDEX, 1, &nullBuffer);
			//unbind sampler
			ID3D11SamplerState* nullSampler = NULL;
			pDeviceContext->PSSetSamplers(TEXTURE_SAMPLE, 1, &nullSampler);

		}

	}
	D3D11MVPParameter mvpParameter;
	mvpParameter.deviceContext = pDeviceContext;
	//unBind MVP 
	d3dParameter->pMVP->UnbindConstantMVP(&mvpParameter);

}
void D3D11Model::Destroy()
{
	for (auto &i : m_modelBuffer)
	{
		SAFE_RELEASE(i.g_pIndexBuffer);
		SAFE_RELEASE(i.g_pVertexBuffer);
	}
	for (auto &i : m_textureSRV)
	{
		SAFE_RELEASE(i.second.diffuseTex.texture);
		SAFE_RELEASE(i.second.normalTex.texture);
	}
	SAFE_RELEASE(m_pConstantLighting);
	SAFE_RELEASE(m_pConstantSkeleton);
	SAFE_RELEASE(m_pSamplerState);
}
D3D11Model::MaterialConstant::MaterialConstant() :diffuseColor(0.f,0.f,0.f,0.f),
specularColor(0.f,0.f,0.f,0.f),
haveTexture(0.f,0.f,0.f),
shiness(0),
metallic(0.f),
roughness(0.5f)
{
	
}