#pragma once
#ifndef _EDITOR_CAMERA_H_
#define _EDITOR_CAMERA_H_
#include "PerspectiveCamera.h"
class EditorCamera : public PerspectiveCamera
{
public:
	EditorCamera(int width, int height);
	virtual ~EditorCamera();
	void Update();
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
};

#endif // !_EDITOR_CAMERA_H_

