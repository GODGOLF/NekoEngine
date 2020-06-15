#ifndef _D3D11_PARTICLE_MODEL_H_
#define _D3D11_PARTICLE_MODEL_H_
#include "D3DModelInF.h"
#include "TextureLoader.h"
#include "Camera.h"
#include "D3D11MVP.h"
#include "D3D11Class.h"
#include "ParticleObj.h"
struct D3D11ParticleModelParameterInitial : public ModelExtraParameter
{
	D3D11Class* pDevice;
	const char* textureFile;
	int particleCount;
};
struct D3D11ParticleModelParameterRender : public ModelExtraParameter
{
	D3D11ParticleModelParameterRender() : pCamera(NULL), pMVP(NULL), pModelInfo(NULL)
	{}

	Camera* pCamera;
	D3D11MVP* pMVP;
	ModelInF* pModelInfo;
	float particleSize;
};
class D3D11ParticleModel :public D3DModelInF
{
public:
	D3D11ParticleModel();
	virtual ~D3D11ParticleModel();
	virtual HRESULT Initial(char* file, ModelExtraParameter* parameter = NULL)  override;
	virtual void Render(void* pDeviceContext, ModelExtraParameter* parameter = NULL)  override;
	virtual void Destroy() override;
private:
	ID3D11Buffer * m_pConstantMaterial;

	ID3D11Buffer* m_pVertexBuffer;

	ID3D11SamplerState* m_pSamplerState;

	Texture::TextureRSV m_diffuseTex;

	ID3D11RasterizerState *m_cullFrontRS;

	int m_particleCount;

	struct ParticleInfo {
		float cTime = 0.f;
		float timeMax =0.f;
		XMFLOAT3 direction = XMFLOAT3(0.f, 0.f, 0.f);
		float speedMax = 0.f;
		float size = 0.f;
	};

	std::vector<ParticleInfo> m_particles;
	struct VertexParticleBuffer
	{
		XMFLOAT4 pos = XMFLOAT4(0, 0, 0, 0);
	};
	std::vector<VertexParticleBuffer> m_vertexData;

private:
	virtual void ComputeTransformation(ID3D11DeviceContext* deviceContext,D3D11ParticleModelParameterRender* data);
protected:
	float FRand(float fMin, float fMax);
	
	
};
#endif // !_D3D11_PARTICLE_MODEL_H_

