#include "DXInclude.h"
#include "D3D11MVP.h"
#include "FunctionHelper.h"
#include "D3D11Class.h"
struct ConMVPBuffer
{
	DirectX::XMMATRIX MVP;
	DirectX::XMMATRIX PROJECTIONMATRIX;
	DirectX::XMMATRIX WORLDMATRIX;
	DirectX::XMMATRIX VIEWMATRIX;
	DirectX::XMMATRIX WORLDINVERSE;
};
D3D11MVP::D3D11MVP() : m_pMVPBuffer(NULL)
{

}
D3D11MVP::~D3D11MVP()
{
	Destroy();
}
HRESULT D3D11MVP::Initial(DXInF* pDevice)
{
	ID3D11Device* dx = ((D3D11Class*)pDevice)->GetDevice();
	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	// Create the constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConMVPBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = dx->CreateBuffer(&bd, nullptr, &m_pMVPBuffer);
	return hr;
}
void D3D11MVP::BindConstantMVP(
	DXInF* pDevice,
	Camera* pCamera,
	DirectX::XMMATRIX defaultMatrix,
	DirectX::XMFLOAT3 pos,
	DirectX::XMFLOAT3 rot,
	DirectX::XMFLOAT3 scale,
	MVP_SHADER_INPUT::VALUE shaderInput)
{
	ID3D11DeviceContext* deviceContext = ((D3D11Class*)pDevice)->GetDeviceContext();
	UpdateContantMVP(deviceContext, pCamera, defaultMatrix, pos, rot, scale, shaderInput);
}
void D3D11MVP::BindConstantMVP(
	void* deviceContext,
	Camera* pCamera,
	DirectX::XMMATRIX defaultMatrix,
	DirectX::XMFLOAT3 pos,
	DirectX::XMFLOAT3 rot,
	DirectX::XMFLOAT3 scale,
	MVP_SHADER_INPUT::VALUE shaderInput)
{
	UpdateContantMVP((ID3D11DeviceContext*)deviceContext, pCamera, defaultMatrix, pos, rot, scale, shaderInput);
}
void D3D11MVP::UnbindConstantMVP(DXInF* pDevice, MVP_SHADER_INPUT::VALUE shaderInput)
{
	ID3D11DeviceContext* dx = ((D3D11Class*)pDevice)->GetDeviceContext();
	ID3D11Buffer* nullBuffer = NULL;
	SetConstantMVP(dx, nullBuffer, shaderInput);
	
}
void D3D11MVP::UnbindConstantMVP(void* deviceContext,
	MVP_SHADER_INPUT::VALUE shaderInput)
{
	ID3D11Buffer* nullBuffer = NULL;

	SetConstantMVP((ID3D11DeviceContext*)deviceContext, nullBuffer, shaderInput);
}
void D3D11MVP::Destroy()
{
	SAFE_RELEASE(m_pMVPBuffer);
}
void D3D11MVP::UpdateContantMVP(ID3D11DeviceContext* deviceContext,
	Camera* pCamera,
	DirectX::XMMATRIX defaultMatrix,
	DirectX::XMFLOAT3 pos,
	DirectX::XMFLOAT3 rot,
	DirectX::XMFLOAT3 scale,
	MVP_SHADER_INPUT::VALUE shaderInput)
{
	ConMVPBuffer cMVP;
	DirectX::XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

	DirectX::XMMATRIX rotateQX = DirectX::XMMatrixRotationQuaternion(DirectXHelper::XMConvertToQuaternion(XMFLOAT3(1, 0, 0), rot.x));
	DirectX::XMMATRIX rotateQY = DirectX::XMMatrixRotationQuaternion(DirectXHelper::XMConvertToQuaternion(XMFLOAT3(0, 1, 0), rot.y));
	DirectX::XMMATRIX rotateQZ = DirectX::XMMatrixRotationQuaternion(DirectXHelper::XMConvertToQuaternion(XMFLOAT3(0, 0, 1), rot.z));
	DirectX::XMMATRIX mMatrix = defaultMatrix;
	DirectX::XMMATRIX mulTrans = DirectX::XMMatrixIdentity();
	mulTrans = XMMatrixMultiply(scaleMatrix, rotateQX);
	mulTrans = XMMatrixMultiply(mulTrans, rotateQY);
	mulTrans = XMMatrixMultiply(mulTrans, rotateQZ);
	mulTrans = XMMatrixMultiply(mulTrans, XMMatrixTranslation(pos.x, pos.y, pos.z));
	mMatrix = XMMatrixMultiply(mMatrix, mulTrans);

	cMVP.MVP = XMMatrixTranspose(mMatrix * pCamera->GetView() * pCamera->GetProjection());
	cMVP.PROJECTIONMATRIX = XMMatrixTranspose(pCamera->GetProjection());
	cMVP.VIEWMATRIX = XMMatrixTranspose(pCamera->GetView());
	cMVP.WORLDMATRIX = XMMatrixTranspose(mMatrix);

	XMMATRIX invernMatrix;

	DirectXHelper::InverseMatrix(mMatrix, invernMatrix);

	cMVP.WORLDINVERSE = XMMatrixTranspose(XMMatrixTranspose(invernMatrix));

	deviceContext->UpdateSubresource(m_pMVPBuffer, 0, nullptr, &cMVP, 0, 0);
	SetConstantMVP(deviceContext, m_pMVPBuffer, shaderInput);
}
void D3D11MVP::SetConstantMVP(ID3D11DeviceContext* deviceContext,ID3D11Buffer* pBuffer,
	MVP_SHADER_INPUT::VALUE shaderInput)
{

	if (shaderInput == MVP_SHADER_INPUT::VERTEX_SHADER)
	{
		deviceContext->VSSetConstantBuffers(MVP_VS_POS, 1, &pBuffer);
	}
	else if (shaderInput == MVP_SHADER_INPUT::PIXEL_SHADER)
	{
		deviceContext->PSSetConstantBuffers(MVP_VS_POS, 1, &pBuffer);
	}
	else if (shaderInput == MVP_SHADER_INPUT::GEO_SHADER)
	{
		deviceContext->GSSetConstantBuffers(MVP_VS_POS, 1, &pBuffer);
	}
	else if (shaderInput == MVP_SHADER_INPUT::HULL_SHADER)
	{
		deviceContext->HSSetConstantBuffers(MVP_VS_POS, 1, &pBuffer);
	}
	else if (shaderInput == MVP_SHADER_INPUT::CS_SHADER)
	{
		deviceContext->CSSetConstantBuffers(MVP_VS_POS, 1, &pBuffer);
	}
}
void D3D11MVP::BindConstantMVP(
	void* deviceContext,
	Camera* pCamera,
	MVP_SHADER_INPUT::VALUE shaderInput)
{
	BindConstantMVP(deviceContext, pCamera,
		XMMatrixIdentity(),
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(0, 0, 0),
		shaderInput);
}