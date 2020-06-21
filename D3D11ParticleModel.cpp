#include "DXInclude.h"
#include "D3D11ParticleModel.h"

#define TIME_DELTA	0.01f
#define SHADER_TYPE_ID				1
#define DIFFUSE_TEXTURE_INDEX		0
#define MATERIAL_CB_INDEX			2
#define TEXTURE_SAMPLE				0
struct MaterialConstant
{
	XMFLOAT4 diffuseColor;
	XMFLOAT3 haveTexture;
	float shaderTypeID;
	XMFLOAT3 camPos;
	float size;
	
};


D3D11ParticleModel::D3D11ParticleModel() :
	m_pConstantMaterial(NULL),
	m_pSamplerState(NULL),
	m_pVertexBuffer(NULL),
	m_cullFrontRS(NULL),
	m_particleCount(0)
{

}
D3D11ParticleModel::~D3D11ParticleModel()
{
	Destroy();
}
HRESULT D3D11ParticleModel::Initial(char* file, ModelExtraParameter* parameter)
{
	D3D11ParticleModelParameterInitial* pParameter = (D3D11ParticleModelParameterInitial*)parameter;

	m_particleCount = pParameter->particleCount;

	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	ID3D11Device* device = pParameter->pDevice->GetDevice();
	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampDesc, &m_pSamplerState);
	if (FAILED(hr)) {
		return hr;
	}
	

	//light constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MaterialConstant);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bd, nullptr, &m_pConstantMaterial);
	if (FAILED(hr))
		return hr;
	//load texture
	hr = Texture::LoadTexture(pParameter->pDevice, pParameter->textureFile, m_diffuseTex);
	if (FAILED(hr))
		return hr;

	//intial data
	m_vertexData.resize(pParameter->particleCount);
	D3D11_SUBRESOURCE_DATA InitData;
	//ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &m_vertexData[0];
	//vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VertexParticleBuffer) * pParameter->particleCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = device->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr))
		return hr;



	D3D11_RASTERIZER_DESC descRast = {
		D3D11_FILL_SOLID,
		D3D11_CULL_FRONT,
		FALSE,
		D3D11_DEFAULT_DEPTH_BIAS,
		D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE,
		FALSE,
		FALSE,
		FALSE
	};
	descRast.DepthBias = 90;
	descRast.SlopeScaledDepthBias = 2.f;
	descRast.DepthBiasClamp = 0.0f;
	descRast.DepthClipEnable = TRUE;
	hr = device->CreateRasterizerState(&descRast, &m_cullFrontRS);
	if (FAILED(hr))
		return hr;

	m_particles.resize(m_particleCount);

	return S_OK;

	
}
void D3D11ParticleModel::Render(void* pDeviceContext, ModelExtraParameter* parameter)
{
	D3D11ParticleModelParameterRender* d3dParameter = (D3D11ParticleModelParameterRender*)parameter;


	ID3D11DeviceContext* deviceContext = (ID3D11DeviceContext*)pDeviceContext;

	ParticleObj* pParticle = (ParticleObj*)d3dParameter->pModelInfo;

	MaterialConstant material = MaterialConstant();
	material.diffuseColor = pParticle->color;
	material.shaderTypeID = SHADER_TYPE_ID;
	material.haveTexture = XMFLOAT3(m_diffuseTex.texture != NULL ? 1.f:0.f, 0.f, 0.f);
	material.size = pParticle->size;
	
	XMStoreFloat3(&material.camPos, d3dParameter->pCamera->GetPosition());

	deviceContext->UpdateSubresource(m_pConstantMaterial, 0, nullptr, &material, 0, 0);
	//do computation
	ComputeTransformation(deviceContext, d3dParameter);

	//bind MVP buffer
	d3dParameter->pMVP->BindConstantMVP(pDeviceContext, d3dParameter->pCamera,
		XMMatrixIdentity(),
		d3dParameter->pModelInfo->position,
		d3dParameter->pModelInfo->GetQuaternion(),
		d3dParameter->pModelInfo->scale);
	//bind MVP buffer
	d3dParameter->pMVP->BindConstantMVP(pDeviceContext, d3dParameter->pCamera,
		XMMatrixIdentity(),
		d3dParameter->pModelInfo->position,
		d3dParameter->pModelInfo->GetQuaternion(),
		d3dParameter->pModelInfo->scale,MVP_SHADER_INPUT::GEO_SHADER);

	deviceContext->PSSetShaderResources(DIFFUSE_TEXTURE_INDEX, 1, &m_diffuseTex.texture);

	deviceContext->PSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &m_pConstantMaterial);
	deviceContext->GSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &m_pConstantMaterial);
	deviceContext->PSSetSamplers(TEXTURE_SAMPLE, 1, &m_pSamplerState);

	UINT stride = sizeof(VertexParticleBuffer);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->Draw(m_particleCount, 0);

	ID3D11ShaderResourceView* texture = NULL;
	deviceContext->PSSetShaderResources(DIFFUSE_TEXTURE_INDEX, 1, &texture);
	//unbind sampler
	ID3D11SamplerState* nullSampler = NULL;
	deviceContext->PSSetSamplers(TEXTURE_SAMPLE, 1, &nullSampler);
	d3dParameter->pMVP->UnbindConstantMVP(deviceContext);
	d3dParameter->pMVP->UnbindConstantMVP(deviceContext, MVP_SHADER_INPUT::GEO_SHADER);

	ID3D11Buffer* nullBuffer = NULL;
	deviceContext->PSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &nullBuffer);
	deviceContext->GSSetConstantBuffers(MATERIAL_CB_INDEX, 1, &nullBuffer);
}	
void D3D11ParticleModel::Destroy()
{
	SAFE_RELEASE(m_pConstantMaterial);
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_diffuseTex.texture);
	SAFE_RELEASE(m_cullFrontRS);
}
float D3D11ParticleModel::FRand(float fMin, float fMax)
{
	float f = (float)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}
void D3D11ParticleModel::ComputeTransformation(ID3D11DeviceContext* deviceContext,D3D11ParticleModelParameterRender* data)
{
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	deviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	ParticleObj* pParticle = (ParticleObj*)data->pModelInfo;
	for (int i = 0; i < m_particleCount; i++)
	{
		if (m_particles[i].cTime >= m_particles[i].timeMax)
		{
			m_particles[i].speedMax = FRand(0.1f, pParticle->speed);

			m_particles[i].timeMax = pParticle->lifeTime;
			
			m_particles[i].cTime = 0.f;

			XMVECTOR direction = XMLoadFloat3(&pParticle->direction);

			XMFLOAT3 fAxis = XMFLOAT3(FRand(-1.0f, 1.0f), FRand(-1.0f, 1.0f), FRand(-1.0f, 1.0f));


			XMVECTOR rot = XMQuaternionRotationAxis(XMLoadFloat3(&fAxis),FRand(-pParticle->radius,pParticle->radius));
			
			XMVECTOR result = XMVector3Rotate(direction, rot);
			
			XMStoreFloat3(&m_particles[i].direction,result);
			
			m_vertexData[i].pos = XMFLOAT4(0, 0, 0, 0);
		}
		m_particles[i].cTime += TIME_DELTA;
		m_vertexData[i].pos.x += m_particles[i].direction.x* m_particles[i].speedMax * TIME_DELTA;
		m_vertexData[i].pos.y += m_particles[i].direction.y* m_particles[i].speedMax * TIME_DELTA;
		m_vertexData[i].pos.z += m_particles[i].direction.z* m_particles[i].speedMax * TIME_DELTA;
	}
	memcpy_s(MappedResource.pData, sizeof(m_vertexData) * m_particleCount, &m_vertexData[0], sizeof(m_vertexData)* m_particleCount);
	deviceContext->Unmap(m_pVertexBuffer, 0);
}