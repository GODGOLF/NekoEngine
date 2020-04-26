
#include "DXInclude.h"
#include "D3D11Shader.h"
#include "FunctionHelper.h"
#include "D3D11Class.h"

D3D11Shader::D3D11Shader()
{

}
D3D11Shader::~D3D11Shader()
{
	Destroy();
}

void D3D11Shader::PreRender(DXInF* pDevice)
{

	ID3D11DeviceContext* pD3DDeviceContext = ((D3D11Class*)pDevice)->GetDeviceContext();
	//set Shader
	pD3DDeviceContext->VSSetShader(m_shader.g_pVertexShader, nullptr, 0);
	pD3DDeviceContext->PSSetShader(m_shader.g_pPixelShader, nullptr, 0);
	pD3DDeviceContext->GSSetShader(m_shader.g_pGeometryShader, nullptr, 0);
	pD3DDeviceContext->DSSetShader(m_shader.g_pDomainShader, nullptr, 0);
	pD3DDeviceContext->HSSetShader(m_shader.g_pHullShader, nullptr, 0);
	pD3DDeviceContext->CSSetShader(m_shader.g_ComputerShader, nullptr, 0);
	// Set the input layout
	pD3DDeviceContext->IASetInputLayout(m_shader.g_pVertexLayout);
}
void D3D11Shader::PostRender(DXInF* pDevice)
{
	ID3D11DeviceContext* pD3DDeviceContext = ((D3D11Class*)pDevice)->GetDeviceContext();

	ID3D11VertexShader*     nullVertexShader	= nullptr;
	ID3D11PixelShader*      nullPixelShader		= nullptr;
	ID3D11GeometryShader*   nullGeoShader		= nullptr;
	ID3D11HullShader*       nullHullShader		= nullptr;
	ID3D11DomainShader*     nullDomainShader	= nullptr;
	ID3D11InputLayout*      nullInputLayout		= nullptr;
	ID3D11ComputeShader*	nullIComputerShader = nullptr;
	pD3DDeviceContext->VSSetShader(nullVertexShader,nullptr, 0);
	pD3DDeviceContext->PSSetShader(nullPixelShader,	nullptr, 0);
	pD3DDeviceContext->GSSetShader(nullGeoShader, nullptr, 0);
	pD3DDeviceContext->DSSetShader(nullDomainShader, nullptr, 0);
	pD3DDeviceContext->HSSetShader(nullHullShader, nullptr, 0);
	pD3DDeviceContext->CSSetShader(nullIComputerShader, nullptr, 0);
	// Set the input layout
	pD3DDeviceContext->IASetInputLayout(nullInputLayout);
}
void D3D11Shader::PreRender(void* pDeviceContext)
{

	ID3D11DeviceContext* pD3DDeviceContext = (ID3D11DeviceContext*)pDeviceContext;

	//set Shader
	pD3DDeviceContext->VSSetShader(m_shader.g_pVertexShader, nullptr, 0);
	pD3DDeviceContext->PSSetShader(m_shader.g_pPixelShader, nullptr, 0);
	pD3DDeviceContext->GSSetShader(m_shader.g_pGeometryShader, nullptr, 0);
	pD3DDeviceContext->DSSetShader(m_shader.g_pDomainShader, nullptr, 0);
	pD3DDeviceContext->HSSetShader(m_shader.g_pHullShader, nullptr, 0);
	pD3DDeviceContext->CSSetShader(m_shader.g_ComputerShader, nullptr, 0);
	// Set the input layout
	pD3DDeviceContext->IASetInputLayout(m_shader.g_pVertexLayout);
}
void D3D11Shader::PostRender(void* pDeviceContext)
{
	ID3D11DeviceContext* pD3DDeviceContext = (ID3D11DeviceContext*)pDeviceContext;

	ID3D11VertexShader*     nullVertexShader = nullptr;
	ID3D11PixelShader*      nullPixelShader = nullptr;
	ID3D11GeometryShader*   nullGeoShader = nullptr;
	ID3D11HullShader*       nullHullShader = nullptr;
	ID3D11DomainShader*     nullDomainShader = nullptr;
	ID3D11InputLayout*      nullInputLayout = nullptr;
	ID3D11ComputeShader*	nullIComputerShader = nullptr;
	pD3DDeviceContext->VSSetShader(nullVertexShader, nullptr, 0);
	pD3DDeviceContext->PSSetShader(nullPixelShader, nullptr, 0);
	pD3DDeviceContext->GSSetShader(nullGeoShader, nullptr, 0);
	pD3DDeviceContext->DSSetShader(nullDomainShader, nullptr, 0);
	pD3DDeviceContext->HSSetShader(nullHullShader, nullptr, 0);
	pD3DDeviceContext->CSSetShader(nullIComputerShader, nullptr, 0);
	// Set the input layout
	pD3DDeviceContext->IASetInputLayout(nullInputLayout);
}

//template<typename T>
//HRESULT D3D11Shader<T>::Initial(DXInF* pDevice, char* file, T layout, SHADER_MODE mode)
//{
//
//}
void D3D11Shader::Destroy()
{
	SAFE_RELEASE(m_shader.g_pGeometryShader);
	SAFE_RELEASE(m_shader.g_pPixelShader);
	SAFE_RELEASE(m_shader.g_pVertexLayout);
	SAFE_RELEASE(m_shader.g_pVertexShader);
	SAFE_RELEASE(m_shader.g_pHullShader);
	SAFE_RELEASE(m_shader.g_pDomainShader);
	SAFE_RELEASE(m_shader.g_ComputerShader);
}
HRESULT D3D11Shader::Initial(DXInF* pDevice, char* file, ShaderLayout* layout, SHADER_MODE::VALUE mode)
{
	HRESULT hr = S_FALSE;

	ID3D11Device* pD3DDevice = ((D3D11Class*)pDevice)->GetDevice();

	D3D11ShaderLayout* pLayout = NULL;
	if (layout == NULL)
	{
		if (mode == SHADER_MODE::VS_MODE
			|| mode == SHADER_MODE::VS_PS_MODE
			|| mode == SHADER_MODE::VS_PS_HS_DS_MODE
			|| mode == SHADER_MODE::VS_PS_GS_MODE)
		{
			return S_FALSE;
		}
		
	}
	else 
	{
		pLayout = (D3D11ShaderLayout*)layout;
	}
	

	wstring wFile = DirectXHelper::ConvertStringToWstring(string(file));
	switch (mode)
	{
	case SHADER_MODE::VS_MODE:
		hr = LoadVertexShader(wFile.c_str(), pD3DDevice, pLayout->layout, m_shader);
		break;
	case SHADER_MODE::PS_MODE:
		hr = LoadPixelShader(wFile.c_str(), pD3DDevice, m_shader);
		break;
	case SHADER_MODE::VS_PS_MODE:
		hr = LoadVertexShader(wFile.c_str(), pD3DDevice, pLayout->layout, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = LoadPixelShader(wFile.c_str(), pD3DDevice, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		break;
	case SHADER_MODE::VS_PS_HS_DS_MODE:
		hr = LoadVertexShader(wFile.c_str(), pD3DDevice, pLayout->layout, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = LoadPixelShader(wFile.c_str(), pD3DDevice, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = LoadHullAndDomainShader(wFile.c_str(), pD3DDevice, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		
		break;
	case SHADER_MODE::VS_PS_GS_MODE:
		hr = LoadGeometricShader(wFile.c_str(), pD3DDevice, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = LoadVertexShader(wFile.c_str(), pD3DDevice, pLayout->layout, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = LoadPixelShader(wFile.c_str(), pD3DDevice, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		break;
	case SHADER_MODE::CS_MODE:
	{
		hr = LoadCSShader(wFile.c_str(), pD3DDevice, m_shader);
	}
	break;
	case SHADER_MODE::VS_GS_MODE:
	{
		hr = LoadGeometricShader(wFile.c_str(), pD3DDevice, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = LoadVertexShader(wFile.c_str(), pD3DDevice, pLayout->layout, m_shader);
		if (FAILED(hr))
		{
			return hr;
		}

	}
	break;
	default:
		break;
	}
	return hr;
}
HRESULT D3D11Shader::LoadVertexShader(const WCHAR* file, ID3D11Device* pDevice, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, Shader &output, string mainName)
{
	HRESULT result;
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	result = CompileShaderFromFile(file,mainName.c_str(), "vs_5_0", &pVSBlob);
	std::string message = DirectXHelper::ConvertWstringToString(std::wstring(file));
	message += " file The VS file cannot be compiled.  Please run this executable from the directory that contains the FX file.";
	if (FAILED(result))
	{
		MessageBox(nullptr,
			message.c_str(), "Error", MB_OK);
		return result;
	}
	// Create the vertex shader
	result = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &output.g_pVertexShader);
	if (FAILED(result))
	{
		pVSBlob->Release();
		return result;
	}
	// Create the input layout
	if (layout.size() >0) {
		result = pDevice->CreateInputLayout(&layout[0], layout.size(), pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(), &output.g_pVertexLayout);

	}
	pVSBlob->Release();
	return S_OK;
}
HRESULT D3D11Shader::LoadPixelShader(const WCHAR* file, ID3D11Device* pDevice, Shader &output, string mainName)
{
	HRESULT result;
	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	result = CompileShaderFromFile(file, mainName.c_str(), "ps_5_0", &pPSBlob);
	std::string message = DirectXHelper::ConvertWstringToString(std::wstring(file));
	message += " file The PS file cannot be compiled.  Please run this executable from the directory that contains the FX file.";
	if (FAILED(result))
	{
		MessageBox(nullptr,
			message.c_str(), "Error", MB_OK);
		return result;
	}

	// Create the pixel shader
	result = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &output.g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(result))
		return result;
	return S_OK;
}
HRESULT D3D11Shader::LoadGeometricShader(const WCHAR* file, ID3D11Device* pDevice, Shader &output, string mainName)
{
	// Compile the geometry shader
	HRESULT result;
	ID3DBlob* pGSBlob = nullptr;
	result = CompileShaderFromFile(file, "GSMain", "gs_5_0", &pGSBlob);
	std::string message = DirectXHelper::ConvertWstringToString(std::wstring(file));
	message += " file The GS file cannot be compiled.  Please run this executable from the directory that contains the FX file.";
	if (FAILED(result))
	{
		MessageBox(nullptr,
			message.c_str(), "Error", MB_OK);
		return result;
	}

	// Create the pixel shader
	result = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &output.g_pGeometryShader);
	pGSBlob->Release();
	if (FAILED(result))
		return result;
	return S_OK;
}
HRESULT D3D11Shader::LoadCSShader(const WCHAR* file, ID3D11Device* device, Shader &output, string mainName)
{
	HRESULT result;
	// Compile the vertex shader
	ID3DBlob* pCSBlob = nullptr;
	result = CompileShaderFromFile(file, "CSMain", "cs_5_0", &pCSBlob);
	std::string message = DirectXHelper::ConvertWstringToString(std::wstring(file));
	message += " file The CS file cannot be compiled.  Please run this executable from the directory that contains the FX file.";
	if (FAILED(result))
	{
		MessageBox(nullptr,
			message.c_str(), "Error", MB_OK);
		return result;
	}
	// Create the pixel shader
	result = device->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &output.g_ComputerShader);
	pCSBlob->Release();
	if (FAILED(result))
		return result;
	return S_OK;
}
HRESULT D3D11Shader::LoadHullAndDomainShader(const WCHAR* file, ID3D11Device* device, Shader &result)
{
	HRESULT hr;
	// Compile the hull shader
	ID3DBlob* pHSBlob = nullptr;
	hr = CompileShaderFromFile(file, "HSMain", "hs_5_0", &pHSBlob);
	std::string message = DirectXHelper::ConvertWstringToString(std::wstring(file));
	message += " file The HS file cannot be compiled.  Please run this executable from the directory that contains the FX file.";
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			message.c_str(), "Error", MB_OK);
		return hr;
	}
	// Create the hull shader
	hr = device->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &result.g_pHullShader);
	message = DirectXHelper::ConvertWstringToString(std::wstring(file));
	message += " file The HS file cannot be compiled.  Please run this executable from the directory that contains the FX file.";
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			message.c_str(), "Error", MB_OK);
		return hr;
	}
	pHSBlob->Release();

	// Compile the domain shader
	ID3DBlob* pDSBlob = nullptr;
	hr = CompileShaderFromFile(file, "DSMain", "ds_5_0", &pDSBlob);
	message = DirectXHelper::ConvertWstringToString(std::wstring(file));
	message += " file The HS file cannot be compiled.  Please run this executable from the directory that contains the FX file.";
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			message.c_str(), "Error", MB_OK);
		return hr;
	}

	// Create the Domain shader
	hr = device->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &result.g_pDomainShader);
	pDSBlob->Release();
	if (FAILED(hr))
		return hr;
	return hr;
}
HRESULT D3D11Shader::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}