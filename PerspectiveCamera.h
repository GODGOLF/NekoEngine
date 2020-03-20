#pragma once

#ifndef _PERSPECTIVE_CAMERA_H_
#define _PERSPECTIVE_CAMERA_H_

#include "Camera.h"

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(int width,
		int height, 
		float degree,
		DirectX::XMFLOAT3 upVector,
		DirectX::XMFLOAT3 camPos,
		DirectX::XMFLOAT3 targetPos,
		float near,
		float far);
	virtual ~PerspectiveCamera();
	float GetFOV()						const;
	float GetAspectRadio()				const;
private:
	float m_aspectRadio;
	float m_FOV;
};
#endif // !_PROJECTION_CAMERA_H_

