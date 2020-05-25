#ifndef _D3D11_TERRAIN_MODEL_H_
#define _D3D11_TERRAIN_MODEL_H_
#include "D3D11PlaneModel.h"

struct D3D11TerrainModelParameterInitial : public D3D11PlaneModelParameterInitial
{
	const char* diffuseTextureFile;
};

struct D3D11TerrainModelParameterRender : public D3D11PlaneModelParameterRender
{

};

class D3D11TerrainModel : public D3D11PlaneModel
{
public:
	D3D11TerrainModel();
	virtual ~D3D11TerrainModel();
	virtual HRESULT Initial(char* file, ModelExtraParameter* parameter = NULL)  override;
	virtual void Render(void* pDeviceContext, ModelExtraParameter* parameter = NULL)  override;
	virtual void Destroy() override;
private:
	Texture::TextureRSV m_diffuseTex;
	Texture::TextureRSV m_normalTex;
	Texture::TextureRSV m_heightMapTex;
	ID3D11SamplerState* m_pSamplerState;
	ID3D11Buffer*  m_pConstantHeightMap;
};
#endif