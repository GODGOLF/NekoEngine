#pragma once
#ifndef _D3D11_MODEL_H_
#define _D3D11_MODEL_H_
#define MAX_SKELETON 50
#include "D3DModelInF.h"
#include "TextureLoader.h"
#include "FBXLoader.h"
#include <vector>
#include <map>
#include "D3D11Class.h"
#include <string>

struct D3D11ModelParameterInitial : ModelExtraParameter
{
	FbxManager* fbxManager;
	D3D11Class* device;
};

class D3D11Model :public D3DModelInF
{
public:
	D3D11Model();
	virtual ~D3D11Model();
	HRESULT Initial(char* file, ModelExtraParameter* parameter =NULL)  override;
	void Render(DXInF* deviceContext, ModelExtraParameter* parameter=NULL)  override;
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
	};

	ID3D11Buffer*  g_pConstantLighting;

	struct ConstantSkeleton {
		XMMATRIX matrix[MAX_SKELETON];
	};
	ID3D11Buffer*  g_pConstantSkeleton;
	
	std::vector<VertexBuffer> m_modelBuffer;
	std::map<std::string, MaterialTexture> m_textureSRV;

	FBXLoader m_model;
};


#endif // !_D3D11_MODEL_H_



