#ifndef _D3D11_SKYBOX_MODEL_H_
#define _D3D11_SKYBOX_MODEL_H_
#include "D3DModelInF.h"
#include "TextureLoader.h"
#include "D3D11Class.h"
#include "D3D11MVP.h"
#include "ModelInF.h"
#include <string>
struct D3D11SkyBoxModelParameterInitial : public ModelExtraParameter
{
	D3D11Class* pDevice;
	std::vector<const char*> textureFile;
	XMFLOAT4 diffuseColor;
	float size;
};
struct D3D11SkyBoxModelParameterRender : public ModelExtraParameter
{
	D3D11SkyBoxModelParameterRender() : pCamera(NULL), pMVP(NULL), pModelInfo(NULL)
	{}
	Camera* pCamera;
	D3D11MVP* pMVP;
	ModelInF* pModelInfo;
	D3D_PRIMITIVE_TOPOLOGY drawType;
};

class D3D11SkyBoxModel :public D3DModelInF
{
public:
	D3D11SkyBoxModel();
	virtual ~D3D11SkyBoxModel();
	virtual HRESULT Initial(char* file, ModelExtraParameter* parameter = NULL)  override;
	virtual void Render(void* pDeviceContext, ModelExtraParameter* parameter = NULL)  override;
	virtual void Destroy() override;
private:


	ID3D11Buffer*  m_pConstantMaterial;

	ID3D11Buffer*  m_pConstantTessa;

	ID3D11Buffer* m_pVertexBuffer;

	ID3D11Buffer* m_pIndexBuffer;

	ID3D11SamplerState* m_pSamplerState;

	Texture::TextureRSV m_diffuseTex;

	DirectX::XMFLOAT4 m_diffuseColor;

	ID3D11RasterizerState *m_cullFrontRS;

private:
	void UpdateTesseration(ID3D11DeviceContext* pDeviceContext, D3D11SkyBoxModelParameterRender* pParameter);

};


#endif // !_D3D11_MODEL_H_


