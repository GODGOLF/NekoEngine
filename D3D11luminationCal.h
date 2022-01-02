#pragma once
#ifndef _D3D11_HDR_H_
#define _D3D11_HDR_H_
#include "PostFXRenderInF.h"
#include "D3D11Shader.h"
struct D3D11PostFXLumInitialP : PostFXRenderParameter
{
	int width;
	int height;
};
struct D3D11PostFXLumRenderP : PostFXRenderParameter
{

};
class D3D11luminationCal : public PostFXRenderInF
{
public:
	D3D11luminationCal();
	~D3D11luminationCal();
	virtual HRESULT Initial(DXInF* pD3D, PostFXRenderParameter* parameter) override;
	virtual void Generate(DXInF* pD3D, PostFXRenderParameter* parameter) override;
	virtual void Destroy() override;
private:
	D3D11Shader m_avLuminane1DShader;
	D3D11Shader m_avLuminaneResultShader;
	
	ID3D11Buffer* m_outputAvg1DBuffer;
	ID3D11Buffer* m_avg1DCS;
	ID3D11UnorderedAccessView* m_outputAvg1DUAV;
	ID3D11ShaderResourceView* m_outputAvg1DSRV;

	ID3D11Buffer* m_avgResultBuffer;
	ID3D11UnorderedAccessView* m_avgResultUAV;
	ID3D11ShaderResourceView* m_avgResultSRV;

	ID3D11Buffer* m_prevAvgLumBuffer;
	ID3D11UnorderedAccessView* m_prevAvgLumUAV;
	ID3D11ShaderResourceView* m_prevAvgLumSRV;

	UINT m_nDownScaleGroups;
};
#endif // _D3D11_HDR_H_
