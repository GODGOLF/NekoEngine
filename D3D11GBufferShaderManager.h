#ifndef _D3D11_GBUFFER_SHADER_MANAGER_H_
#define _D3D11_GBUFFER_SHADER_MANAGER_H_
#include "ShaderManagerInF.h"
#include "D3DShaderInF.h"
struct SHADER_TYPE
{
	enum VALUE
	{
		MODEL_SHADER,
		TERRAIN_SHADER,
		OCEAN_SHADER,
		PARTICLE_SHADER,
		SHADER_COUNT
	};
};

class D3D11GBufferShaderManager : public ShaderManagerInF
{
public:
	D3D11GBufferShaderManager();
	virtual ~D3D11GBufferShaderManager();
	HRESULT Initial(DXInF* pDevice) override;
	void Destroy() override;
	void GetShader(SHADER_TYPE::VALUE value, D3DShaderInF** output);
private:
	D3DShaderInF* m_pShader[SHADER_TYPE::SHADER_COUNT];
};
#endif // !_D3D11_GBUFFER_SHADER_MANAGER_H_

