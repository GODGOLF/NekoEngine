#include "Camera.h"
#include "FunctionHelper.h"

const DirectX::XMVECTOR G_FORWORD_VECTOR = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
const DirectX::XMVECTOR G_RIGHT_VECTOR = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

Camera::Camera()
{
	m_view = DirectX::XMMatrixIdentity();
	m_projection = DirectX::XMMatrixIdentity();
	m_camPos = DirectX::XMVectorSet(0, 0, 0, 0);
}
Camera::~Camera()
{

}

DirectX::XMMATRIX Camera::GetView()
{
	return m_view;
}
DirectX::XMMATRIX Camera::GetProjection()
{
	return m_projection;
}
DirectX::XMVECTOR Camera::GetPosition()
{
	return m_camPos;
}

float Camera::GetNearValue()
{
	return m_nearValue;
}
float Camera::GetFarValue()
{
	return m_farValue;
}
inline DirectX::XMVECTOR Camera::GetWorldRight()
{
	DirectX::XMFLOAT4X4 vCameraWorld; // World matrix of the camera (inverse of the view matrix)
	DirectX::XMMATRIX mCameraWorld;
	DirectXHelper::InverseMatrix(m_view, mCameraWorld);
	XMStoreFloat4x4(&vCameraWorld, mCameraWorld);
	return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&vCameraWorld._11));
}
inline DirectX::XMVECTOR Camera::GetWorldUp()
{
	DirectX::XMFLOAT4X4 vCameraWorld; // World matrix of the camera (inverse of the view matrix)
	DirectX::XMMATRIX mCameraWorld;
	DirectXHelper::InverseMatrix(m_view, mCameraWorld);
	XMStoreFloat4x4(&vCameraWorld, mCameraWorld);
	return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&vCameraWorld._21));
}
inline DirectX::XMVECTOR Camera::GetWorldAhead()
{
	DirectX::XMFLOAT4X4 vCameraWorld; // World matrix of the camera (inverse of the view matrix)
	DirectX::XMMATRIX mCameraWorld;
	DirectXHelper::InverseMatrix(m_view, mCameraWorld);
	XMStoreFloat4x4(&vCameraWorld, mCameraWorld);
	return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&vCameraWorld._31));
}
float Camera::GetFOV() {
	return m_FOV;
}
float Camera::GetAspectRadio() {
	return m_aspectRadio;
}