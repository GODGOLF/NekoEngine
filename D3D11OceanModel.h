#ifndef _D3D11_OCEAN_MODEL_H_
#define _D3D11_OCEAN_MODEL_H_
#include "D3D11PlaneModel.h"

#define WAVE_COUNT	3
struct D3D11OceanModelParameterInitial : public D3D11PlaneModelParameterInitial
{
	const char* normalTexture;
};
struct D3D11WaveInfo
{
	XMFLOAT2 direction = XMFLOAT2(1.f,1.f);
	float waveLength = 10.f;
	float steepness = 0.5f;
};
struct D3D11OceanModelParameterRender : public D3D11PlaneModelParameterRender
{
	float time;
	D3D11WaveInfo waveInfo[3];
	
};

class D3D11OceanModel : public D3D11PlaneModel
{
public:
	D3D11OceanModel();
	virtual ~D3D11OceanModel();
	virtual HRESULT Initial(char* file, ModelExtraParameter* parameter = NULL)  override;
	virtual void Render(void* pDeviceContext, ModelExtraParameter* parameter = NULL)  override;
	virtual void Destroy() override;
private:
	Texture::TextureRSV m_normalTexture;
	ID3D11SamplerState* m_pSamplerState;
	ID3D11Buffer*  m_pConstantOceantMap;
};
#endif