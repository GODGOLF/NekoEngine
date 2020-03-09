/*
DXInF.h

Author: Zhuang Theerapong

*/
#pragma once
#ifndef SCREEN_INTERFACE_H
#define SCREEN_INTERFACE_H

#include "WindowApplication.h"


class ScreenInF
{
public:
	ScreenInF() {};
	virtual ~ScreenInF() {};

	virtual HRESULT OnInit(HWND *hwnd, HINSTANCE hInstance, unsigned int width, unsigned int height) = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender(HWND hwnd) = 0;
	virtual void OnDestroy() = 0;
};

#endif // !SCREEN_INTERFACE_H

