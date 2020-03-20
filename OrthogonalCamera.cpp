#include "OrthogonalCamera.h"

using namespace DirectX;
OrthgonalCamera::OrthgonalCamera(int width,
	int height,
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
	m_projection = XMMatrixOrthographicLH((float)width, (float)height, near, far);
	m_nearValue = near;
	m_farValue = far;
}
OrthgonalCamera::~OrthgonalCamera()
{

}