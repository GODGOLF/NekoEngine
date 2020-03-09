#pragma once
#ifndef _D3D11_MODEL_H_
#define _D3D11_MODEL_H_
#include "D3DModelInF.h"
#include "TextureLoader.h"
#include "FBXLoader.h"
#include <vector>
#include <map>
#include "D3D11Class.h"

struct D3D11ModelParameterInitial : ModelExtraParameter
{
	FbxManager* fbxManager;
	D3D11Class* device;
};

class D3D11Model :public D3DModelInF
{
public:
	HRESULT initial(char* file, ModelExtraParameter* parameter =NULL)  override;
	void render(DXInF* deviceContext, ModelExtraParameter* parameter=NULL)  override;
	void destroy() override;
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
	
	std::vector<VertexBuffer> m_modelBuffer;
	std::map<std::string, MaterialTexture> m_textureSRV;

	FBXLoader m_model;
};


#endif // !_D3D11_MODEL_H_



