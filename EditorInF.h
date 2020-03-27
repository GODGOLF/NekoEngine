/*
EditorInf.h

Author: Zhuang Theerapong

*/
#pragma once

#ifndef _EDITOR_INTERFACE_H_
#define _EDITOR_INTERFACE_H_
#include "Camera.h"
class EditorInf {
public:
	virtual void Render(Camera* camera) = 0;
};
#endif // !_EDITOR_INTERFACE_H_#pragma once
