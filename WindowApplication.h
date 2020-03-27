
/*
WindowApplication.h

Author: Zhuang Theerapong

*/
#ifndef _WINDOWAPPLICATION_H_
#define _WINDOWAPPLICATION_H_
#pragma once
#include <ROApi.h>
#include "ScreenInF.h"

class ScreenInF;

class WindowApplication
{
public:
	static int Run(ScreenInF* pScreen, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hwnd; }


protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hwnd;
};

#endif // !WINDOWAPPLICATION_H#pragma once
