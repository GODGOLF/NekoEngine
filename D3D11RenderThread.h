#pragma once
#ifndef _D3D11_RENDER_THREAD_H_
#define _D3D11_RENDER_THREAD_H_
#include "RenderInF.h"
#include "ThreadHandle.h"
#include "DXInF.h"

struct RenderThreadInitialParameter : Parameter
{
	int width;
	int height;
};
class D3D11RenderThread : public RenderInF, public ThreadHandle
{
public:
	D3D11RenderThread();

	virtual ~D3D11RenderThread();

	virtual HRESULT Initial(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Render(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Update(DXInF* pDevice, Parameter* pParameter = nullptr) override;

	virtual void Destroy() override;

	ID3D11CommandList* GetCommandList();

private:

	virtual void ThreadExcecute() override;

	D3D11_VIEWPORT m_defaultVP;

protected:

	ID3D11DeviceContext* m_deviceContext;

	ID3D11CommandList* m_commanList;

	DXInF* m_pDevice;

	


};

#endif // !_D3D11_RENDER_THREAD_H_
