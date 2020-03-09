#pragma once
#ifndef _DXInF_H
#define _DXInF_H
class DXInF {
public:
	DXInF() {};
	virtual ~DXInF() {};

	virtual HRESULT OnInit(HWND hwnd, UINT width, UINT height) = 0;
	virtual void BeginDraw() = 0;
	virtual void EndDraw() = 0;
	virtual void OnDestroy() = 0;


	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
};

#endif // !_D3DInf_H
