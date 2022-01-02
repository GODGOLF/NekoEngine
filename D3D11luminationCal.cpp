#include "DXInclude.h"
#include "D3D11luminationCal.h"
#include "D3D11Class.h"

#define  LUMINANCE_1D_SHADER		"Data/Shader/FirstLuminanceCS.fx"
#define  LUMINANCE_RESULT_SHADER	"Data/Shader/FirstLuminanceCS.fx"

D3D11luminationCal::D3D11luminationCal() : 
	m_outputAvg1DBuffer(NULL),
	m_avg1DCS(NULL),
	m_outputAvg1DUAV(NULL),
	m_outputAvg1DSRV(NULL),
	m_avgResultBuffer(NULL),
	m_avgResultUAV(NULL),
	m_avgResultSRV(NULL),
	m_prevAvgLumBuffer(NULL),
	m_prevAvgLumUAV(NULL),
	m_prevAvgLumSRV(NULL)
{

}
D3D11luminationCal::~D3D11luminationCal()
{
	Destroy();
}
HRESULT D3D11luminationCal::Initial(DXInF* pD3D, PostFXRenderParameter* parameter)
{
	 
	HRESULT hr = E_FAIL;

	hr = m_avLuminane1DShader.Initial(pD3D, (char*)LUMINANCE_1D_SHADER, NULL, SHADER_MODE::CS_MODE);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create HDR shader file");
#endif // _DEBUG

		return hr;
	}

	hr =m_avLuminaneResultShader.Initial(pD3D, (char*)LUMINANCE_RESULT_SHADER, NULL, SHADER_MODE::CS_MODE);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create HDR shader file");
#endif // _DEBUG
		return hr;
	}

	ID3D11Device* pD3DDevice = ((D3D11Class*)pD3D)->GetDevice();
	D3D11PostFXLumInitialP* pParameter = (D3D11PostFXLumInitialP*)parameter;

	m_nDownScaleGroups = (UINT)ceil((float)(pParameter->width * pParameter->height / 16) / 1024.0f);
	
	D3D11_BUFFER_DESC outputDesc;
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputDesc.ByteWidth = sizeof(float) * m_nDownScaleGroups;
	outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	outputDesc.StructureByteStride = sizeof(float);
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	hr = pD3DDevice->CreateBuffer(&outputDesc, 0, &m_outputAvg1DBuffer);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create HDR buffer");
#endif // _DEBUG
		return hr;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.Flags = 0;
	DescUAV.Buffer.NumElements = outputDesc.ByteWidth / sizeof(float);

	hr = pD3DDevice->CreateUnorderedAccessView(m_outputAvg1DBuffer, &DescUAV, &m_outputAvg1DUAV);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create HDR UAV");
#endif // _DEBUG
		return hr;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_UNKNOWN;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	dsrvd.Buffer.NumElements = m_nDownScaleGroups;
	hr = pD3DDevice->CreateShaderResourceView(m_outputAvg1DBuffer, &dsrvd, &m_outputAvg1DSRV);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create HDR SAV");
#endif // _DEBUG
		return hr;
	}
	
	//second pass
	ZeroMemory(&outputDesc, sizeof(D3D11_BUFFER_DESC));
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputDesc.StructureByteStride = sizeof(float);
	outputDesc.ByteWidth = outputDesc.StructureByteStride;
	outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	hr = pD3DDevice->CreateBuffer(&outputDesc, 0, &m_avgResultBuffer);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create HDR buffer");
#endif // _DEBUG
		return hr;
	}

	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.Flags = 0;
	DescUAV.Buffer.NumElements = outputDesc.ByteWidth / sizeof(float);

	hr = pD3DDevice->CreateUnorderedAccessView(m_avgResultBuffer, &DescUAV, &m_avgResultUAV);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create HDR UAV");
#endif // _DEBUG
		return hr;
	}

	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_UNKNOWN;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	dsrvd.Buffer.NumElements = outputDesc.ByteWidth / sizeof(float);
	hr = pD3DDevice->CreateShaderResourceView(m_avgResultBuffer, &dsrvd, &m_avgResultSRV);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create HDR SRV");
#endif // _DEBUG
		return hr;
	}

	//prev Luminance factor
	ZeroMemory(&outputDesc, sizeof(D3D11_BUFFER_DESC));
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputDesc.StructureByteStride = sizeof(float);
	outputDesc.ByteWidth = outputDesc.StructureByteStride;
	outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	hr = pD3DDevice->CreateBuffer(&outputDesc, 0, &m_prevAvgLumBuffer);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create prev HDR buffer");
#endif // _DEBUG
		return hr;
	}

	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.Flags = 0;
	DescUAV.Buffer.NumElements = outputDesc.ByteWidth / sizeof(float);

	hr = pD3DDevice->CreateUnorderedAccessView(m_prevAvgLumBuffer, &DescUAV, &m_prevAvgLumUAV);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create prev HDR UAV");
#endif // _DEBUG
		return hr;
	}
	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_UNKNOWN;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	dsrvd.Buffer.NumElements = outputDesc.ByteWidth / sizeof(float);
	hr = pD3DDevice->CreateShaderResourceView(m_prevAvgLumBuffer, &dsrvd, &m_prevAvgLumSRV);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		OutputDebugString("Fail to create prev HDR SRV");
#endif // _DEBUG
		return hr;
	}

	return S_OK;
}
void D3D11luminationCal::Generate(DXInF* d3d, PostFXRenderParameter* parameter)
{
	D3D11Class* pD3D11 = (D3D11Class*)d3d;

	ID3D11DeviceContext* pD3DContext = pD3D11->GetDeviceContext();
	m_avLuminane1DShader.PreRender(pD3DContext);

	//pD3DContext.
		
	m_avLuminane1DShader.PostRender(pD3DContext);
}
void D3D11luminationCal::Destroy()
{
	SAFE_RELEASE(m_outputAvg1DBuffer);
	SAFE_RELEASE(m_avg1DCS);
	SAFE_RELEASE(m_outputAvg1DUAV);
	SAFE_RELEASE(m_outputAvg1DSRV);
	SAFE_RELEASE(m_avgResultBuffer);
	SAFE_RELEASE(m_avgResultUAV);
	SAFE_RELEASE(m_avgResultSRV);
	SAFE_RELEASE(m_prevAvgLumBuffer);
	SAFE_RELEASE(m_prevAvgLumUAV);
	SAFE_RELEASE(m_prevAvgLumSRV);
	m_avLuminane1DShader.Destroy();
	m_avLuminaneResultShader.Destroy();
}