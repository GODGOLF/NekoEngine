
#ifndef _ORTHGONAL_CAMERA_H_
#define _ORTHGONAL_CAMERA_H_

#include "Camera.h"

class OrthgonalCamera : public Camera
{
public:
	OrthgonalCamera(int width,
		int height,
		DirectX::XMFLOAT3 upVector,
		DirectX::XMFLOAT3 camPos,
		DirectX::XMFLOAT3 targetPos,
		float near,
		float far);
	virtual ~OrthgonalCamera();
};
#endif // !_PROJECTION_CAMERA_H_#pragma once
