#include "FunctionHelper.h"

void DirectXHelper::InverseMatrix(DirectX::XMMATRIX Input, DirectX::XMMATRIX &output)
{
	DirectX::XMMATRIX A = Input;
	DirectX::XMVECTOR det = XMMatrixDeterminant(A);
	output = XMMatrixInverse(&det, A);
}
DirectX::XMVECTOR DirectXHelper::XMConvertToQuaternion(DirectX::XMFLOAT3 axis, float angle) {
	DirectX::XMVECTOR aV = XMLoadFloat3(&axis);
	return DirectX::XMQuaternionRotationAxis(aV, angle);
}
std::wstring DirectXHelper::ConvertStringToWstring(std::string text)
{
	std::wstring str(text.length(), L' '); // Make room for characters
										   // Copy string to wstring.
	std::copy(text.begin(), text.end(), str.begin());
	
	return str;
}
std::string DirectXHelper::ConvertWstringToString(std::wstring text)
{
	std::string str;
	size_t convertedChars = 0;
	size_t  sizeInBytes = ((text.size() + 1) * 2);
	str.resize((text.size() + 1) * 2);
	wcstombs_s(&convertedChars, &str[0], sizeInBytes, text.c_str(), sizeInBytes);
	return str;
}
DirectX::XMFLOAT3 DirectXHelper::ConvertQuaternionToEulerAngle(DirectX::XMFLOAT4 q)
{
	//x axis
	//float xAxis = atan2(2.f * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);
	////y axis
	//float yAxis = asin(-2.f * (q.x * q.z - q.w * q.y));
	////z axis
	//float zAxis = atan2(2.f * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z);
	//
	//DirectX::XMFLOAT3 eulerAngle = DirectX::XMFLOAT3(xAxis, yAxis, zAxis);
	DirectX::XMFLOAT3 eulerAngle;
	float test = q.x*q.y + q.z*q.w;
	float xAxis;
	if (test >0.499f)
	{
		eulerAngle.z = DirectX::XM_PI / 2.f;
		eulerAngle.y = 2.f * atan2f(q.x, q.w);
		eulerAngle.x = 0;
		return eulerAngle;
	}
	if (test < -0.499) { // singularity at south pole
		eulerAngle.z = -DirectX::XM_PI / 2;
		eulerAngle.y = -2.f * atan2f(q.x, q.w);
		eulerAngle.x = 0;
		return eulerAngle;
	}
	float sqx = q.x*q.x;
	float sqy = q.y*q.y;
	float sqz = q.z*q.z;
	eulerAngle.z = asinf(2 * test);
	eulerAngle.y = atan2f(2 * q.y*q.w - 2 * q.x*q.z, 1 - 2 * sqy - 2 * sqz);
	eulerAngle.x = atan2(2 * q.x*q.w - 2 * q.y*q.z, 1 - 2 * sqx - 2 * sqz);
	
	return eulerAngle;
}