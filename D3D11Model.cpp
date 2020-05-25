#include "DXInclude.h"
#include "D3D11Model.h"
#include "AnimeObj.h"
#include "TextureLoader.h"
#define DIFFUSE_TEXTURE_INDEX 0
#define NORMAL_TEXTURE_INDEX 1
#define TEXTURE_SAMPLE	0

#define SKELETON_MATRIX_CB_INDEX	2
#define MATERIAL_CB_INDEX			3

#define DISPLACEMENT_CB_INDEX		5
#define DISPLACEMENT_TEX_INDEX		2


struct MaterialConstant
{
	MaterialConstant();
	XMFLOAT4 diffuseColor;
	XMFLOAT4 specularColor;
	XMFLOAT3 haveTexture;
	float specExp;
	float  metallic;
	float roughness;
	float pad[2];
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


D3D11Model::D3D11Model() : m_pConstantLighting(NULL),m_pConstantSkeleton(NULL), m_pSamplerState(NULL)
{

}
D3D11Model::~D3D11Model()
{
	Destroy();
}
struct ConstantSkeleton {
	XMMATRIX matrix[MAX_SKELETON];
};

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
					Texture::LoadTexture(d3dParameter->pDevice, j.mat.diffuseTexture, m_textureSRV[j.name].diffuseTex);
				}
				if (strlen(j.mat.normalTexture) != 0)
				{
					Texture::LoadTexture(d3dParameter->pDevice, j.mat.normalTexture, m_textureSRV[j.name].normalTex);
				}
				if (strlen(j.mat.displacementTexture) != 0)
				{
					Texture::LoadTexture(d3dParameter->pDevice, j.mat.displacementTexture, m_textureSRV[j.name].displacementTex);
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
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ConstantSkeleton);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = device->CreateBuffer(&bd, nullptr, &m_pConstantSkeleton);
	if (FAILED(hr))
		return hr;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(DisPlacementMapCB);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = device->CreateBuffer(&bd, nullptr, &m_pConstantDisplacement);
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

	//if have animation,bind data into it;
	UpdateAnimation(pDeviceContext, d3dParameter->pModelInfo);

	

	UINT stride = sizeof(VertexAnime);
	UINT offset = 0;
	for (unsigned int i = 0; i < list->size(); i++) {
		pDeviceContext->IASetVertexBuffers(0, 1, &m_modelBuffer[i].g_pVertexBuffer, &stride, &offset);
		// Set index buffer
		pDeviceContext->IASetIndexBuffer(m_modelBuffer[i].g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		// Set primitive topology
		pDeviceContext->IASetPrimitiveTopology(d3dParameter->drawType);

		//bind MVP buffer
		d3dParameter->pMVP->BindConstantMVP(pDeviceContext,d3dParameter->pCamera,
			list->operator[](i).DefaultMatrix, 
			d3dParameter->pModelInfo->position, 
			d3dParameter->pModelInfo->rotation, 
			d3dParameter->pModelInfo->scale);
		for (auto &j : list->operator[](i).material) {
			
			MaterialConstant material;
			material.diffuseColor = j.mat.diffuseColor;
			material.specularColor = j.mat.specularColor;
			material.specExp = j.mat.shiness;
			material.roughness = d3dParameter->pModelInfo->roughness;
			material.metallic = d3dParameter->pModelInfo->metallic;
			//bind Tesseration 
			UpdateTesseration(pDeviceContext, d3dParameter, (m_textureSRV[j.name].displacementTex.texture != NULL)? 1.f : 0.f);
			pDeviceContext->VSSetConstantBuffers(DISPLACEMENT_CB_INDEX, 1, &m_pConstantDisplacement);
			pDeviceContext->DSSetConstantBuffers(DISPLACEMENT_CB_INDEX, 1, &m_pConstantDisplacement);
			//bind Displacement texture
			if (m_textureSRV[j.name].displacementTex.texture != NULL)
			{
				pDeviceContext->DSSetShaderResources(DISPLACEMENT_TEX_INDEX, 1, &m_textureSRV[j.name].displacementTex.texture);
			}

			XMFLOAT3 haveTex = XMFLOAT3(0, 0, 0);
			if (m_textureSRV[j.name].diffuseTex.texture != NULL) {
				pDeviceContext->PSSetShaderResources(DIFFUSE_TEXTURE_INDEX, 1, &m_textureSRV[j.name].diffuseTex.texture);
				haveTex.x = 1;
			}
			if (m_textureSRV[j.name].normalTex.texture != NULL) {
				pDeviceContext->PSSetShaderResources(NORMAL_TEXTURE_INDEX, 1, &m_textureSRV[j.name].normalTex.texture);
				haveTex.y = 1;
			}
			if (m_model.haveAnimation) {
				pDeviceContext->VSSetConstantBuffers(SKELETON_MATRIX_CB_INDEX, 1, &m_pConstantSkeleton);
			}
			material.haveTexture = haveTex;
			pDeviceContext->UpdateSubresource(m_pConstantLighting, 0, nullptr, &material, 0, 0);
			pDeviceContext->PSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &m_pConstantLighting);
			pDeviceContext->PSSetSamplers(TEXTURE_SAMPLE, 1, &m_pSamplerState);
			pDeviceContext->DSSetSamplers(TEXTURE_SAMPLE, 1, &m_pSamplerState);
			//draw object
			pDeviceContext->DrawIndexed(j.count, j.startIndex,0);
			//unbind texture
			ID3D11ShaderResourceView* texture = NULL;
			pDeviceContext->PSSetShaderResources(DIFFUSE_TEXTURE_INDEX, 1, &texture);
			pDeviceContext->PSSetShaderResources(NORMAL_TEXTURE_INDEX, 1, &texture);
			pDeviceContext->DSSetShaderResources(DISPLACEMENT_TEX_INDEX, 1, &texture);
			//unbind buffer
			ID3D11Buffer* nullBuffer = NULL;
			pDeviceContext->VSSetConstantBuffers(SKELETON_MATRIX_CB_INDEX, 1, &nullBuffer);
			pDeviceContext->VSSetConstantBuffers(DISPLACEMENT_CB_INDEX, 1, &nullBuffer);
			pDeviceContext->DSSetConstantBuffers(DISPLACEMENT_CB_INDEX, 1, &nullBuffer);
			//unbind sampler
			ID3D11SamplerState* nullSampler = NULL;
			pDeviceContext->PSSetSamplers(TEXTURE_SAMPLE, 1, &nullSampler);
			pDeviceContext->DSSetSamplers(TEXTURE_SAMPLE, 1, &nullSampler);

		}

	}
	//unBind MVP 
	d3dParameter->pMVP->UnbindConstantMVP(pDeviceContext);

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
	SAFE_RELEASE(m_pConstantDisplacement);
}
MaterialConstant::MaterialConstant() :diffuseColor(0.f,0.f,0.f,0.f),
specularColor(0.f,0.f,0.f,0.f),
haveTexture(0.f,0.f,0.f),
specExp(0),
metallic(0.1f),
roughness(0.5f)
{
	
}
void D3D11Model::UpdateAnimation(ID3D11DeviceContext* pDeviceContext, ModelInF* pModelInfo)
{
	if(m_model.haveAnimation)
	{
		int animStackIndex = ((AnimeObj*)pModelInfo)->GetAnimationStackIndex();
		long long animTime = ((AnimeObj*)pModelInfo)->GetAnimationTime();
		//if don't selected animation stack, use first of animation stack,but animation is not run
		if (animStackIndex == -1)
		{
			animStackIndex =0;
			animTime = 0;
		}
		if (animTime == -1)
		{
			animTime = 0;
		}
		std::vector<AnimationStack>* animStack = m_model.GetAnimationStacks();

		std::string stackName = animStack->operator[](animStackIndex).name.Buffer();

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pDeviceContext->Map(m_pConstantSkeleton, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

		ConstantSkeleton* cb = (ConstantSkeleton*)MappedResource.pData;

		std::vector<Joint>* pJoint =  m_model.GetJoint();

		long long frameIndex = m_model.ConvertMillisecondToFrameRate(AnimationFrame::e24, animTime);
		
		for (unsigned int i = 0; i < pJoint->size(); i++)
		{
			
			if (pJoint->operator[](i).keyframeAnimation.size() == 0)
			{
				cb->matrix[i] = XMMatrixIdentity();
				continue;
			}
			else if ((unsigned int)frameIndex > pJoint->operator[](i).keyframeAnimation[stackName].size() - 1)
			{
				unsigned int newFrameIndex = pJoint->operator[](i).keyframeAnimation[stackName].size() - 1;
				cb->matrix[i] = XMMatrixTranspose(pJoint->operator[](i).keyframeAnimation[stackName][newFrameIndex].transformMatrix);
			}
			else
			{
				cb->matrix[i] = XMMatrixTranspose(pJoint->operator[](i).keyframeAnimation[stackName][(unsigned int)frameIndex].transformMatrix);
			}
			if (pJoint->operator[](i).keyframeAnimation[stackName].size() > 0)
			{
				int a = pJoint->operator[](i).keyframeAnimation[stackName].size();
				int b = 0;
			}
			
		}
		pDeviceContext->Unmap(m_pConstantSkeleton,0);

		//clear data
		std::string().swap(stackName);
	}
}
FBXLoader* D3D11Model::GetModelData()
{
	return &m_model;
}
void D3D11Model::UpdateTesseration(ID3D11DeviceContext* pDeviceContext,D3D11ModelParameterRender* pParameter, float haveTexture)
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pDeviceContext->Map(m_pConstantDisplacement, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	DisPlacementMapCB* cb = (DisPlacementMapCB*)MappedResource.pData;
	cb->gHeightScale = pParameter->pModelInfo->heightScale;
	cb->gMaxTessDistance = pParameter->pModelInfo->maxTessDistance;
	cb->gMinTessDistance = pParameter->pModelInfo->minTessDistance;
	cb->gMinTessFactor = pParameter->pModelInfo->minTessFactor;
	cb->gMaxTessFactor = pParameter->pModelInfo->maxTessFactor;
	cb->haveDisplacementMap = haveTexture;
	XMStoreFloat3(&cb->gCameraPos, pParameter->pCamera->GetPosition());
	pDeviceContext->Unmap(m_pConstantDisplacement, 0);
}