#pragma once
#ifndef _EDITOR_CAMERA_H_
#define _EDITOR_CAMERA_H_
#include "PerspectiveCamera.h"
#include "KMInputManager.h"
class EditorCamera : public PerspectiveCamera
{
public:
	EditorCamera(int width, int height);
	virtual ~EditorCamera();
	void Update(KMInputManager* key);
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
private:
	float m_moveSpeed;
	float m_zoom;
	float m_yaw;				// Relative to the +z axis.
	float m_pitch;				// Relative to the xz plane.
	float m_turnSpeed;
	DirectX::XMFLOAT2 m_mousePos;
	float m_mouSensitiveX;
	float m_mouSensitiveY;
	float m_maxRadiusPitch;
	float m_minRadiusPitch;
};

#endif // !_EDITOR_CAMERA_H_

