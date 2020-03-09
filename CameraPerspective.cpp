#include "CameraPerspective.h"

CameraPerspective::CameraPerspective(float width,
	float height,
	DirectX::XMFLOAT3 pos,
	DirectX::XMFLOAT3 target,
	float radius,
	DirectX::XMFLOAT3 camUp,
	float nearF,
	float farF)
{
	m_camPos = DirectX::XMLoadFloat3(&pos);
	// Initialize the view matrix
	//XM_PIDIV4
	m_projection = DirectX::XMMatrixPerspectiveFovLH(radius, width / height, nearF, farF);

	m_FOV = radius;
	m_aspectRadio = width / height;
	m_view = DirectX::XMMatrixLookAtLH(m_camPos, XMLoadFloat3(&target), XMLoadFloat3(&camUp));
	
	this->m_nearValue = nearF;
	this->m_farValue = farF;
}