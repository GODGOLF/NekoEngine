#pragma once
#ifndef _DXInF_H
#define _DXInF_H
class DXInF {
public:
	DXInF() {};
	virtual ~DXInF() {};

	virtual HRESULT OnInit(HWND hwnd, UINT width, UINT height) = 0;
	virtual void BindMainRenderTarget() = 0;
	virtual void EndDraw() = 0;
	virtual void OnDestroy() = 0;
};

#endif // !_D3DInf_H
