#pragma once
#ifndef _MVP_INTERFACE_H_
#define _MVP_INTERFACE_H_
#include <DirectXMath.h>
#include "DXInF.h"
#include "Camera.h"
#include <Windows.h>
struct MVP_SHADER_INPUT
{
	enum VALUE {
		VERTEX_SHADER,
		PIXEL_SHADER,
		GEO_SHADER,
		HULL_SHADER,
		DOMAIN_SHADER,
		CS_SHADER
	};
};

class MVPInF
{
public:
	MVPInF() {};
	virtual ~MVPInF() {};
	virtual HRESULT Initial(DXInF* deviceContext) = 0;
	virtual void BindConstantMVP(
		DXInF* deviceContext,
		Camera* pCamera,
		DirectX::XMMATRIX defaultMatrix,
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT4 rot,
		DirectX::XMFLOAT3 scale,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) =0;
	virtual void BindConstantMVP(
		void* deviceContext,
		Camera* pCamera,
		DirectX::XMMATRIX defaultMatrix,
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT4 rot,
		DirectX::XMFLOAT3 scale,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) = 0;
	virtual void BindConstantMVP(
		void* deviceContext,
		Camera* pCamera,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) = 0;
	virtual void UnbindConstantMVP(void* deviceContext,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) = 0;
	virtual void UnbindConstantMVP(DXInF* pDevice, 
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) =0;
	virtual void Destroy() = 0;
};
#endif // !_MVP_INTERFACE_H_
