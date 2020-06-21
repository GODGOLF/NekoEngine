#include "ModelInF.h"
#include "FunctionHelper.h"
std::string ModelInF::GetModelIndex() const
{
	return m_modelIndex;
}
DirectX::XMFLOAT3 ModelInF::GetRotation()
{
	return m_rotation;
}
void ModelInF::SetRotation(DirectX::XMFLOAT4 val)
{
	m_quaternion = val;
	m_rotation = DirectXHelper::ConvertQuaternionToEulerAngle(val);
}
void ModelInF::SetRotation(DirectX::XMFLOAT3 axis, float angle)
{
	m_rotation.x = axis.x * angle;
	m_rotation.y = axis.y * angle;
	m_rotation.z = axis.z * angle;
	DirectX::XMStoreFloat4(&m_quaternion, DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&axis), angle));
}
void ModelInF::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
	DirectX::XMStoreFloat4(&m_quaternion, DirectX::XMQuaternionRotationRollPitchYaw(x, y, z));
}
DirectX::XMFLOAT4 ModelInF::GetQuaternion()
{
	return m_quaternion;
}