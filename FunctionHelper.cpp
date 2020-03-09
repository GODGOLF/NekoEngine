#include "FunctionHelper.h"

inline void DirectXHelper::InverseMatrix(DirectX::XMMATRIX Input, DirectX::XMMATRIX &output)
{
	DirectX::XMMATRIX A = Input;
	DirectX::XMVECTOR det = XMMatrixDeterminant(A);
	output = XMMatrixInverse(&det, A);
}
inline DirectX::XMVECTOR DirectXHelper::XMConvertToQuaternion(DirectX::XMFLOAT3 axis, float angle) {
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
	char *ch = (char *)malloc(sizeInBytes);
	wcstombs_s(&convertedChars, ch, sizeInBytes,text.c_str(), sizeInBytes);
	str.append(ch);
	return str;
}