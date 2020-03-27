#pragma once
#ifndef _D3D11_MODEL_H_
#define _D3D11_MODEL_H_
#define MAX_SKELETON 50
#include "D3DModelInF.h"
#include "TextureLoader.h"
#include "FBXLoader.h"
#include "D3D11Class.h"
#include "D3D11MVP.h"
#include "ModelInF.h"
#include <string>
#include <vector>
#include <map>
struct D3D11ModelParameterInitial : ModelExtraParameter
{
	FbxManager* pFbxManager;
	D3D11Class* pDevice;
};
struct D3D11ModelParameterRender : ModelExtraParameter
{
	Camera* pCamera;
	D3D11MVP* pMVP;
	ModelInF* pModelInfo;
};

class D3D11Model :public D3DModelInF
{
public:
	D3D11Model();
	virtual ~D3D11Model();
	HRESULT Initial(char* file, ModelExtraParameter* parameter =NULL)  override;
	void Render(void* pDeviceContext, ModelExtraParameter* parameter=NULL)  override;
	void Destroy() override;
private:
	struct MaterialTexture
	{
		Texture::TextureRSV diffuseTex;
		Texture::TextureRSV normalTex;
	};
	struct VertexBuffer
	{
		ID3D11Buffer* g_pVertexBuffer = nullptr;
		ID3D11Buffer* g_pIndexBuffer = nullptr;
	};
	struct MaterialConstant
	{
		MaterialConstant();
		XMFLOAT4 diffuseColor;
		XMFLOAT4 specularColor;
		XMFLOAT3 haveTexture;
		float shiness;
		float  metallic;
		float roughness;
		float pad[2];
	};

	ID3D11Buffer*  m_pConstantLighting;

	struct ConstantSkeleton {
		XMMATRIX matrix[MAX_SKELETON];
	};
	ID3D11Buffer*  m_pConstantSkeleton;
	
	std::vector<VertexBuffer> m_modelBuffer;
	std::map<std::string, MaterialTexture> m_textureSRV;

	FBXLoader m_model;

	ID3D11SamplerState* m_pSamplerState;
};


#endif // !_D3D11_MODEL_H_



