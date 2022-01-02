#pragma once
#ifndef _FUNCTION_HELPER_H_
#define _FUNCTION_HELPER_H_
#include <DirectXMath.h>
#include <string>
#include <typeinfo>
#include <Windows.h>
namespace DirectXHelper
{
	void InverseMatrix(DirectX::XMMATRIX Input, DirectX::XMMATRIX& output);
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

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}
}
#endif // !_FUNCTION_HELPER_H_


