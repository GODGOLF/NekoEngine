#include "DXInclude.h"
#include "D3D11PointLightRender.h"
#include "D3D11Class.h"

#define SHADER_FILE "Data/Shader/PointLight.fx"

#define DEPTH_TEXTURE				0
#define COLOR_SPEC_TEXTURE			1
#define NORMAL_TEXTURE				2
#define SPEC_POWER_TEXTURE			3
#define PREPARE_FOR_UNPACK_INDEX	1
#define POINT_LIGHT_CB_INDEX		2
#define VIEW_PROJECTION_MATRIX		3

#define DEPTH_TEXTURE_3D			4
#define COLOR_SPEC_TEXTURE_3D		5
#define NORMAL_TEXTURE_3D			6
#define SPEC_POWER_TEXTURE_3D		7

struct CBPointLightPixel
{
	XMFLOAT3 PointLightPos;
	float PointLightRangeRcp;
	XMFLOAT3 PointColor;
	float pad1;
	XMFLOAT2 LightPerspectiveValues;
	float Intensity;
	float transparent;
};

struct CBPointLightDomain
{
	XMMATRIX LightProjection;
};

D3D11PointLightRender::D3D11PointLightRender()
	:m_pProjectionLightCB(NULL), m_pPointLightCB(NULL), m_pSamplerState(NULL)
{
	
}
D3D11PointLightRender::~D3D11PointLightRender()
{
	Destroy();
}

HRESULT D3D11PointLightRender::Initial(DXInF* pDevice)
{
	HRESULT hr;
	D3D11ShaderLayout shaderLayout;
	shaderLayout.layout =
	{
	};

	hr = m_shader.Initial(pDevice, (char*)SHADER_FILE, &shaderLayout, SHADER_MODE::VS_PS_HS_DS_MODE);
	if (FAILED(hr))
	{
		return hr;
	}

	ID3D11Device* pd3dDevice = ((D3D11Class*)pDevice)->GetDevice();
	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 0;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerState);
	if (FAILED(hr))
	{
		return hr;
	}
	// Create point light constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CBPointLightPixel);
	hr = pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pPointLightCB);
	if (hr != S_OK) {
		return hr;
	}
	// Create perspective constant buffers
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CBPointLightDomain);
	hr = pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pProjectionLightCB);
	if (hr != S_OK) {
		return hr;
	}
	return S_OK;
}
void D3D11PointLightRender::Render(void* pDeviceContext,
	LightObjInF* pObj,
	Camera* pCamera,
	LightParameter* extraParameter)
{
	ID3D11DeviceContext* pd3dDeviceContext = (ID3D11DeviceContext*)pDeviceContext;

	PointLightRenderParameter* parameter = (PointLightRenderParameter*)extraParameter;
	PointLightObj* pDirObj = (PointLightObj*)pObj;
	// Fill the directional and ambient values constant buffer
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dDeviceContext->Map(m_pPointLightCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CBPointLightPixel * pPointLightValuesCB = (CBPointLightPixel*)MappedResource.pData;
	pPointLightValuesCB->PointLightPos = pDirObj->Position;
	pPointLightValuesCB->PointLightRangeRcp = 1 / pDirObj->Radius;
	pPointLightValuesCB->PointColor = XMFLOAT3(pDirObj->Color.x, pDirObj->Color.y, pDirObj->Color.z);
	pPointLightValuesCB->Intensity = pDirObj->Intensity;
	pPointLightValuesCB->transparent = parameter->transparent ? 1.f : 0.f;
	pd3dDeviceContext->Unmap(m_pPointLightCB, 0);
	//Fill the projection view matrix
	pd3dDeviceContext->Map(m_pProjectionLightCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CBPointLightDomain * pPersiveViewValuesCB = (CBPointLightDomain*)MappedResource.pData;
	XMMATRIX mLightWorldScale = XMMatrixScaling(pDirObj->Radius, pDirObj->Radius, pDirObj->Radius);
	XMMATRIX mLightWorldTrans = XMMatrixTranslation(pDirObj->Position.x, pDirObj->Position.y, pDirObj->Position.z);
	XMMATRIX mView = pCamera->GetView();
	XMMATRIX mProj = pCamera->GetProjection();
	pPersiveViewValuesCB->LightProjection = XMMatrixTranspose(mLightWorldScale * mLightWorldTrans * mView * mProj);
	pd3dDeviceContext->Unmap(m_pProjectionLightCB, 0);
	
	//add shader
	m_shader.PreRender(pDeviceContext);
	//add texture
	if (parameter->transparent)
	{
		pd3dDeviceContext->PSSetShaderResources(DEPTH_TEXTURE_3D, 1, &parameter->depthStencilDSV);
		pd3dDeviceContext->PSSetShaderResources(COLOR_SPEC_TEXTURE_3D, 1, &parameter->colorSRV);
		pd3dDeviceContext->PSSetShaderResources(NORMAL_TEXTURE_3D, 1, &parameter->normalSRV);
		pd3dDeviceContext->PSSetShaderResources(SPEC_POWER_TEXTURE_3D, 1, &parameter->specPowerSRV);
	}
	else
	{
		pd3dDeviceContext->PSSetShaderResources(DEPTH_TEXTURE, 1, &parameter->depthStencilDSV);
		pd3dDeviceContext->PSSetShaderResources(COLOR_SPEC_TEXTURE, 1, &parameter->colorSRV);
		pd3dDeviceContext->PSSetShaderResources(NORMAL_TEXTURE, 1, &parameter->normalSRV);
		pd3dDeviceContext->PSSetShaderResources(SPEC_POWER_TEXTURE, 1, &parameter->specPowerSRV);
	}
	

	pd3dDeviceContext->PSSetConstantBuffers(POINT_LIGHT_CB_INDEX,1,&m_pPointLightCB);
	pd3dDeviceContext->DSSetConstantBuffers(VIEW_PROJECTION_MATRIX, 1, &m_pProjectionLightCB);
	pd3dDeviceContext->PSSetConstantBuffers(PREPARE_FOR_UNPACK_INDEX, 1, &parameter->GBufferUnpackCB);

	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	pd3dDeviceContext->Draw(2, 0);
	
	//clear texture and package
	ID3D11ShaderResourceView* textureNULL = NULL;
	if (parameter->transparent)
	{
		pd3dDeviceContext->PSSetShaderResources(DEPTH_TEXTURE_3D, 1, &textureNULL);
		pd3dDeviceContext->PSSetShaderResources(COLOR_SPEC_TEXTURE_3D, 1, &textureNULL);
		pd3dDeviceContext->PSSetShaderResources(NORMAL_TEXTURE_3D, 1, &textureNULL);
		pd3dDeviceContext->PSSetShaderResources(SPEC_POWER_TEXTURE_3D, 1, &textureNULL);
	}
	else 
	{
		pd3dDeviceContext->PSSetShaderResources(DEPTH_TEXTURE, 1, &textureNULL);
		pd3dDeviceContext->PSSetShaderResources(COLOR_SPEC_TEXTURE, 1, &textureNULL);
		pd3dDeviceContext->PSSetShaderResources(NORMAL_TEXTURE, 1, &textureNULL);
		pd3dDeviceContext->PSSetShaderResources(SPEC_POWER_TEXTURE, 1, &textureNULL);
	}
	

	ID3D11Buffer* bufferNull = NULL;
	pd3dDeviceContext->PSSetConstantBuffers(POINT_LIGHT_CB_INDEX, 1, &bufferNull);
	pd3dDeviceContext->DSSetConstantBuffers(VIEW_PROJECTION_MATRIX, 1, &bufferNull);
	pd3dDeviceContext->PSSetConstantBuffers(PREPARE_FOR_UNPACK_INDEX, 1, &bufferNull);

	m_shader.PostRender(pDeviceContext);
}
void D3D11PointLightRender::Destroy()
{
	m_shader.Destroy();
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pPointLightCB);
	SAFE_RELEASE(m_pProjectionLightCB);
}