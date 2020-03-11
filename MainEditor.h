#ifndef MAIN_EDITOR
#define MAIN_EDITOR
#include<Windows.h>
#include <wrl.h>
#include <shellapi.h>
#include "ScreenInF.h"
#include "NekoEngine.h"
#include "ObjSceneInF.h"

class MainEditor : public ScreenInF
{
public:
	MainEditor();
	virtual HRESULT OnInit(HWND *hwnd, HINSTANCE hInstance, unsigned int width, unsigned int height) override;
	virtual void OnUpdate()  override;
	virtual void OnRender(HWND hWnd) override;
	virtual void OnDestroy() override;
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
	
private:
	NekoEngine m_engine;
	ObjScene* m_objScene;

	Camera m_mainCamera;
};
#endif // !D3D_SYSTEM

