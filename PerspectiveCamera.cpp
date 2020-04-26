#include "PerspectiveCamera.h"

using namespace DirectX;
PerspectiveCamera::PerspectiveCamera(int width,
	int height, 
	float degree, 
	DirectX::XMFLOAT3 upVector, 
	DirectX::XMFLOAT3 camPos, 
	DirectX::XMFLOAT3 targetPos,
	float near,
	float far)
{
	m_camPos = XMLoadFloat3(&camPos);
	XMVECTOR vTargetPos = XMLoadFloat3(&targetPos);
	XMVECTOR vUp = XMLoadFloat3(&upVector);
	m_view = XMMatrixLookAtLH(m_camPos, vTargetPos, vUp);
	m_FOV = XMConvertToRadians(degree);
	m_aspectRadio = (float)width / (float)height;
	m_projection = XMMatrixPerspectiveFovLH(m_FOV, m_aspectRadio, 0.1f, 1000.0f);
	m_nearValue = near;
	m_farValue = far;
}
PerspectiveCamera::~PerspectiveCamera()
{

}
float PerspectiveCamera::GetFOV()  const {
	return m_FOV;
}
float PerspectiveCamera::GetAspectRadio()  const {
	return m_aspectRadio;
}