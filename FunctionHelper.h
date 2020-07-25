#pragma once
#include <DirectXMath.h>
#include <string>
#include <typeinfo>
namespace DirectXHelper
{
	void InverseMatrix(DirectX::XMMATRIX Input, DirectX::XMMATRIX &output);
	DirectX::XMVECTOR XMConvertToQuaternion(DirectX::XMFLOAT3 axis, float angle);
	template< typename Base, typename T>
	bool instanceof(const T* ptr) {
		return dynamic_cast<const Base*>(ptr) != nullptr;
	}
	template<class T, class K>
	inline bool instantOfByTypeId(const K k)
	{
		if (typeid(T) == typeid(*k))
		{
			return true;
		}
		return false;
	}
	std::wstring ConvertStringToWstring(std::string text);
	std::string ConvertWstringToString(std::wstring text);
	DirectX::XMFLOAT3 ConvertQuaternionToEulerAngle(DirectX::XMFLOAT4 quaterion);
}
