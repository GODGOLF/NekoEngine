/*
MVPClass.h

Author: Zhuang Theerapong

*/
#pragma once
#ifndef _MVP_CLASS_H_
#define _MVP_CLASS_H_
#define MVP_VS_POS	(0)
#include "MVPInF.h"
class D3D11MVP : MVPInF{
private:
public:
	D3D11MVP();
	virtual ~D3D11MVP();
	HRESULT Initial(DXInF* deviceContext) override;
	void BindConstantMVP(
		DXInF* pDevice,
		Camera* pCamera,
		DirectX::XMMATRIX defaultMatrix,
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT4 rot,
		DirectX::XMFLOAT3 scale,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) override;
	void BindConstantMVP(
		void* deviceContext,
		Camera* pCamera,
		DirectX::XMMATRIX defaultMatrix,
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT4 rot,
		DirectX::XMFLOAT3 scale,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER);
	void BindConstantMVP(
		void* deviceContext,
		Camera* pCamera,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) override;
		
	void UnbindConstantMVP(DXInF* pDevice, 
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) override;
	void UnbindConstantMVP(void* deviceContext,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER) override;
	void Destroy() override;
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
	


private:
	ID3D11Buffer* m_pMVPBuffer;
	void UpdateContantMVP(ID3D11DeviceContext* deviceContext, 
		Camera* pCamera,
		DirectX::XMMATRIX defaultMatrix,
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT4 rot,
		DirectX::XMFLOAT3 scale,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER);
	void SetConstantMVP(ID3D11DeviceContext* deviceContext, ID3D11Buffer* pBuffer,
		MVP_SHADER_INPUT::VALUE shaderInput = MVP_SHADER_INPUT::VERTEX_SHADER);
};

#endif // !_EFFECT_INF_H


