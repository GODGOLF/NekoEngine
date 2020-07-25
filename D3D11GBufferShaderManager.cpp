#include "DXInclude.h"
#include "D3D11GBufferShaderManager.h"
#include "D3D11Shader.h"

#define G_BUFFER_FILE				"Data/Shader/GBuffer.fx"
#define G_BUFFER_TERRAIN_FILE		"Data/Shader/TerrainGBuffer.fx"
#define G_BUFFER_OCEAN_FILE			"Data/Shader/OceanGBuffer.fx"		
#define G_BUFFER_PARTICLE_FILE		"Data/Shader/ParticleGBuffer.fx"	
#define C_T_SHADER	"Data/Shader/TransparentGBuffer.fx"	

D3D11GBufferShaderManager::D3D11GBufferShaderManager() 
{
	memset(&m_pShader[0], NULL, SHADER_TYPE::SHADER_COUNT);
}
D3D11GBufferShaderManager::~D3D11GBufferShaderManager()
{
	Destroy();
}

HRESULT D3D11GBufferShaderManager::Initial(DXInF* pDevice)
{
	HRESULT hr;
	D3D11ShaderLayout shaderLayout;
	shaderLayout.layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 80,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	//initial shader
	m_pShader[SHADER_TYPE::MODEL_SHADER] = new D3D11Shader();
	hr = m_pShader[SHADER_TYPE::MODEL_SHADER]->Initial(pDevice, (char*)G_BUFFER_FILE, &shaderLayout, SHADER_MODE::VS_PS_HS_DS_GS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}
	
	ZeroMemory(&shaderLayout, sizeof(D3D11ShaderLayout));
	shaderLayout.layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_pShader[SHADER_TYPE::TERRAIN_SHADER] = new D3D11Shader();
	hr = m_pShader[SHADER_TYPE::TERRAIN_SHADER]->Initial(pDevice, (char*)G_BUFFER_TERRAIN_FILE, &shaderLayout, SHADER_MODE::VS_PS_HS_DS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}
	ZeroMemory(&shaderLayout, sizeof(D3D11ShaderLayout));
	shaderLayout.layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_pShader[SHADER_TYPE::OCEAN_SHADER] = new D3D11Shader();
	hr = m_pShader[SHADER_TYPE::OCEAN_SHADER]->Initial(pDevice, (char*)G_BUFFER_OCEAN_FILE, &shaderLayout, SHADER_MODE::VS_PS_HS_DS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}
	ZeroMemory(&shaderLayout, sizeof(D3D11ShaderLayout));
	shaderLayout.layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_pShader[SHADER_TYPE::PARTICLE_SHADER] = new D3D11Shader();
	hr = m_pShader[SHADER_TYPE::PARTICLE_SHADER]->Initial(pDevice, (char*)G_BUFFER_PARTICLE_FILE, &shaderLayout, SHADER_MODE::VS_PS_GS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}
	ZeroMemory(&shaderLayout, sizeof(D3D11ShaderLayout));
	m_pShader[SHADER_TYPE::TRANSPARENT_CLASSIFICATION_SHADER] = new D3D11Shader();
	hr = m_pShader[SHADER_TYPE::TRANSPARENT_CLASSIFICATION_SHADER]->Initial(pDevice, (char*)C_T_SHADER, NULL, SHADER_MODE::CS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void D3D11GBufferShaderManager::Destroy()
{
	for (unsigned int i = 0; i < SHADER_TYPE::SHADER_COUNT; i++)
	{
		if (m_pShader[i])
		{
			delete m_pShader[i];
			m_pShader[i] = NULL;
		}
	}
}
void D3D11GBufferShaderManager::GetShader(SHADER_TYPE::VALUE value, D3DShaderInF** output)
{
	if (value == SHADER_TYPE::SHADER_COUNT)
	{
		*output = NULL;
		return;
	}
	*output =  m_pShader[value];
	
}