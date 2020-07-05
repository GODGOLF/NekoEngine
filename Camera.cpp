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

DirectX::XMMATRIX Camera::GetView() const
{
	return m_view;
}
DirectX::XMMATRIX Camera::GetProjection() const
{
	return m_projection;
}
DirectX::XMVECTOR Camera::GetPosition() const
{
	return m_camPos;
}

float Camera::GetNearValue() const
{
	return m_nearValue;
}
float Camera::GetFarValue() const
{
	return m_farValue;
}
DirectX::XMVECTOR Camera::GetWorldRight() const
{
	DirectX::XMFLOAT4X4 vCameraWorld; // World matrix of the camera (inverse of the view matrix)
	DirectX::XMMATRIX mCameraWorld;
	DirectXHelper::InverseMatrix(m_view, mCameraWorld);
	XMStoreFloat4x4(&vCameraWorld, mCameraWorld);
	return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&vCameraWorld._11));
}
DirectX::XMVECTOR Camera::GetWorldUp() const
{
	DirectX::XMFLOAT4X4 vCameraWorld; // World matrix of the camera (inverse of the view matrix)
	DirectX::XMMATRIX mCameraWorld;
	DirectXHelper::InverseMatrix(m_view, mCameraWorld);
	XMStoreFloat4x4(&vCameraWorld, mCameraWorld);
	return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&vCameraWorld._21));
}
DirectX::XMVECTOR Camera::GetWorldAhead() const
{
	DirectX::XMFLOAT4X4 vCameraWorld; // World matrix of the camera (inverse of the view matrix)
	DirectX::XMMATRIX mCameraWorld;
	DirectXHelper::InverseMatrix(m_view, mCameraWorld);
	XMStoreFloat4x4(&vCameraWorld, mCameraWorld);
	return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&vCameraWorld._31));
}
void Camera::SetView(DirectX::XMMATRIX val)
{
	m_view = val;
	DirectX::XMFLOAT4X4 vCameraWorld; // World matrix of the camera (inverse of the view matrix)
	DirectX::XMMATRIX mCameraWorld;
	DirectXHelper::InverseMatrix(m_view, mCameraWorld);
	XMStoreFloat4x4(&vCameraWorld, mCameraWorld);
	m_camPos = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&vCameraWorld._41));

}
void Camera::SetProjection(DirectX::XMMATRIX val)
{
	m_projection = val;
}
