#include "DXInclude.h"
#include "D3D11SpotLightRender.h"
#include "D3D11Class.h"

#define SHADER_FILE "Data/Shader/SpotLight.fx"

#define DEPTH_TEXTURE				0
#define COLOR_SPEC_TEXTURE			1
#define NORMAL_TEXTURE				2
#define SPEC_POWER_TEXTURE			3
#define PREPARE_FOR_UNPACK_INDEX	1
#define Spot_LIGHT_CB_INDEX			2
#define VIEW_PROJECTION_MATRIX		3

struct CBSpotLightPixel
{
	XMFLOAT3 spotLightPos;
	float spotLightRangeRcp;
	XMFLOAT3 spotDirToLight;
	float spotCosOuterCone;
	XMFLOAT3 spotColor;
	float sspotCosConeAttRange;
	float Intensity;
	float pad[3];
};

struct CBSpotLightDomain
{
	XMMATRIX LightProjection;
	float sinAngle;
	float cosAngle;
	float pad[2];
};

D3D11SpotLightRender::D3D11SpotLightRender()
	:m_pProjectionLightCB(NULL), m_pSpotLightCB(NULL), m_pSamplerState(NULL)
{

}
D3D11SpotLightRender::~D3D11SpotLightRender()
{
	Destroy();
}

HRESULT D3D11SpotLightRender::Initial(DXInF* pDevice)
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
	// Create Spot light constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CBSpotLightPixel);
	hr = pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pSpotLightCB);
	if (hr != S_OK) {
		return hr;
	}
	// Create perspective constant buffers
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CBSpotLightDomain);
	hr = pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pProjectionLightCB);
	if (hr != S_OK) {
		return hr;
	}
	return S_OK;
}
void D3D11SpotLightRender::Render(void* pDeviceContext,
	LightObjInF* pObj,
	Camera* pCamera,
	LightParameter* extraParameter)
{
	ID3D11DeviceContext* pd3dDeviceContext = (ID3D11DeviceContext*)pDeviceContext;


	SpotLightRenderParameter* parameter = (SpotLightRenderParameter*)extraParameter;
	SpotLightObj* pDirObj = (SpotLightObj*)pObj;

	float fCosInnerAngle = cosf(XMConvertToRadians(pDirObj->InnerAngle));
	float fSinOuterAngle = sinf(XMConvertToRadians(pDirObj->OuterAngle));
	float fCosOuterAngle = cosf(XMConvertToRadians(pDirObj->OuterAngle));

	// Fill the directional and ambient values constant buffer
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dDeviceContext->Map(m_pSpotLightCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CBSpotLightPixel * pSpotLightValuesCB = (CBSpotLightPixel*)MappedResource.pData;
	pSpotLightValuesCB->spotLightPos = pDirObj->Position;
	pSpotLightValuesCB->spotLightRangeRcp = 1.f / pDirObj->Radius;
	pSpotLightValuesCB->spotDirToLight = pDirObj->Direction;
	pSpotLightValuesCB->spotColor = XMFLOAT3(pDirObj->Color.x, pDirObj->Color.y, pDirObj->Color.z);
	pSpotLightValuesCB->spotCosOuterCone = fCosOuterAngle;
	pSpotLightValuesCB->sspotCosConeAttRange = fCosInnerAngle - fCosOuterAngle;
	pSpotLightValuesCB->Intensity = pDirObj->Intensity;
	pd3dDeviceContext->Unmap(m_pSpotLightCB, 0);
	//Fill the projection view matrix
	pd3dDeviceContext->Map(m_pProjectionLightCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CBSpotLightDomain * pProjectViewValuesCB = (CBSpotLightDomain*)MappedResource.pData;
	XMMATRIX mLightWorldScale = XMMatrixScaling(pDirObj->Radius, pDirObj->Radius, pDirObj->Radius);
	XMMATRIX mView = pCamera->GetView();
	XMMATRIX mProj = pCamera->GetProjection();

	XMMATRIX mLightMatrixTranRotate = XMMatrixIdentity();

	// transtate and rotate
	XMVECTOR vUp = (pDirObj->Direction.y > 0.9f || pDirObj->Direction.y < -0.9f) ? XMVectorSet(0, 0, pDirObj->Direction.y, 0) : XMVectorSet(0, 1, 0, 0);

	XMVECTOR vDir = XMLoadFloat3(&pDirObj->Direction);

	XMVECTOR vRight = XMVector3Cross(vUp, vDir);

	vRight = XMVector3Normalize(vRight);

	vUp = XMVector3Cross(vDir, vRight);

	vUp = XMVector3Normalize(vUp);

	mLightMatrixTranRotate.r[0] = vRight;
	mLightMatrixTranRotate.r[1] = vUp;
	mLightMatrixTranRotate.r[2] = vDir;
	mLightMatrixTranRotate.r[3] = XMVectorSet(pDirObj->Position.x, pDirObj->Position.y, pDirObj->Position.z, 1.f);

	pProjectViewValuesCB->LightProjection = XMMatrixTranspose(mLightWorldScale * mLightMatrixTranRotate * mView * mProj);

	pProjectViewValuesCB->sinAngle = fSinOuterAngle;
	pProjectViewValuesCB->cosAngle = fCosOuterAngle;
	pd3dDeviceContext->Unmap(m_pProjectionLightCB, 0);

	//add shader
	m_shader.PreRender(pDeviceContext);
	//add texture
	pd3dDeviceContext->PSSetShaderResources(DEPTH_TEXTURE, 1, &parameter->depthStencilDSV);
	pd3dDeviceContext->PSSetShaderResources(COLOR_SPEC_TEXTURE, 1, &parameter->colorSRV);
	pd3dDeviceContext->PSSetShaderResources(NORMAL_TEXTURE, 1, &parameter->normalSRV);
	pd3dDeviceContext->PSSetShaderResources(SPEC_POWER_TEXTURE, 1, &parameter->specPowerSRV);

	pd3dDeviceContext->PSSetConstantBuffers(Spot_LIGHT_CB_INDEX, 1, &m_pSpotLightCB);
	pd3dDeviceContext->DSSetConstantBuffers(VIEW_PROJECTION_MATRIX, 1, &m_pProjectionLightCB);
	pd3dDeviceContext->PSSetConstantBuffers(PREPARE_FOR_UNPACK_INDEX, 1, &parameter->GBufferUnpackCB);

	pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	pd3dDeviceContext->Draw(1, 0);

	//clear texture and package
	ID3D11ShaderResourceView* textureNULL = NULL;
	pd3dDeviceContext->PSSetShaderResources(DEPTH_TEXTURE, 1, &textureNULL);
	pd3dDeviceContext->PSSetShaderResources(COLOR_SPEC_TEXTURE, 1, &textureNULL);
	pd3dDeviceContext->PSSetShaderResources(NORMAL_TEXTURE, 1, &textureNULL);
	pd3dDeviceContext->PSSetShaderResources(SPEC_POWER_TEXTURE, 1, &textureNULL);

	ID3D11Buffer* bufferNull = NULL;
	pd3dDeviceContext->PSSetConstantBuffers(Spot_LIGHT_CB_INDEX, 1, &bufferNull);
	pd3dDeviceContext->DSSetConstantBuffers(VIEW_PROJECTION_MATRIX, 1, &bufferNull);
	pd3dDeviceContext->PSSetConstantBuffers(PREPARE_FOR_UNPACK_INDEX, 1, &bufferNull);

	m_shader.PostRender(pDeviceContext);
}
void D3D11SpotLightRender::Destroy()
{
	m_shader.Destroy();
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pSpotLightCB);
	SAFE_RELEASE(m_pProjectionLightCB);
}