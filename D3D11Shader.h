#pragma once
#ifndef _D3D11_SHADER_H_
#define _D3D11_SHADER_H_
#include "D3DShaderInF.h"

struct Shader {
	Shader() :g_pVertexShader(NULL), 
		g_pPixelShader(NULL), 
		g_pGeometryShader(NULL),
		g_pHullShader(NULL),
		g_pDomainShader(NULL),
		g_pVertexLayout(NULL),
		g_ComputerShader(NULL)
	{

	}
	ID3D11VertexShader*     g_pVertexShader = nullptr;
	ID3D11PixelShader*      g_pPixelShader = nullptr;
	ID3D11GeometryShader*     g_pGeometryShader = nullptr;
	ID3D11HullShader*		g_pHullShader = nullptr;
	ID3D11DomainShader*		g_pDomainShader = nullptr;
	ID3D11InputLayout*      g_pVertexLayout = nullptr;
	ID3D11ComputeShader* g_ComputerShader = nullptr;
};

struct D3D11ShaderLayout : ShaderLayout
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
};
class D3D11Shader :public D3DShaderInF
{
public:
	explicit D3D11Shader();
	virtual ~D3D11Shader();
	virtual	 HRESULT Initial(DXInF* pDevice, char* file, ShaderLayout* layout, SHADER_MODE::VALUE mode);
	virtual void PreRender(DXInF* pDevice) override;
	virtual void PostRender(DXInF* pDevice) override;
	virtual void PreRender(void* pDeviceContext) override;
	virtual void PostRender(void* pDeviceContext) override;
	virtual void Destroy() override;
private:
	HRESULT LoadVertexShader(const WCHAR* file, ID3D11Device* pDevice, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, Shader &output, string mainName = "VSMain");
	HRESULT LoadPixelShader(const WCHAR* file, ID3D11Device* pDevice, Shader &output, string mainName = "PSMain");
	HRESULT LoadGeometricShader(const WCHAR* file, ID3D11Device* pDevice, Shader &output, string mainName = "GSMain");
	HRESULT LoadCSShader(const WCHAR* file, ID3D11Device* device, Shader &output, string mainName = "CSMain");
	HRESULT LoadHullAndDomainShader(const WCHAR* file, ID3D11Device* device, Shader &output);
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	Shader m_shader;

};


#endif // !_D3D11_SHADER_H_

