#pragma once
#ifndef _D3D11_MODEL_H_
#define _D3D11_MODEL_H_
#define MAX_SKELETON 200
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
	D3D11ModelParameterRender() : pCamera(NULL), pMVP(NULL), pModelInfo(NULL)
	{}
	Camera* pCamera;
	D3D11MVP* pMVP;
	ModelInF* pModelInfo;
	D3D_PRIMITIVE_TOPOLOGY drawType;
};

class D3D11Model :public D3DModelInF
{
public:
	D3D11Model();
	virtual ~D3D11Model();
	HRESULT Initial(char* file, ModelExtraParameter* parameter =NULL)  override;
	void Render(void* pDeviceContext, ModelExtraParameter* parameter=NULL)  override;
	void Destroy() override;
	FBXLoader* GetModelData();
private:
	struct MaterialTexture
	{
		Texture::TextureRSV diffuseTex;
		Texture::TextureRSV normalTex;
		Texture::TextureRSV displacementTex;
	};
	struct VertexBuffer
	{
		ID3D11Buffer* g_pVertexBuffer = nullptr;
		ID3D11Buffer* g_pIndexBuffer = nullptr;
	};
	
	ID3D11Buffer*  m_pConstantMaterial;

	
	ID3D11Buffer*  m_pConstantSkeleton;

	ID3D11Buffer*  m_pConstantDisplacement;
	
	std::vector<VertexBuffer> m_modelBuffer;
	std::map<std::string, MaterialTexture> m_textureSRV;


	FBXLoader m_model;

	ID3D11SamplerState* m_pSamplerState;

private:
	void UpdateAnimation(ID3D11DeviceContext* pDeviceContext, ModelInF* pModelInfo);
	void UpdateTesseration(ID3D11DeviceContext* pDeviceContext,D3D11ModelParameterRender* pParameter, float haveTexture);
};


#endif // !_D3D11_MODEL_H_



