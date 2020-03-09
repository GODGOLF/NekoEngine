#pragma once
#ifndef _CAMERA_PERSPECTIVE_H_
#define _CAMERA_PERSPECTIVE_H_
#include "Camera.h"

class CameraPerspective : public Camera
{
public:
	CameraPerspective(float width,
		float height,
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT3 target,
		float radius,
		DirectX::XMFLOAT3 camUp = DirectX::XMFLOAT3(0, 1, 0),
		float nearF = 0.01,
		float farF = 1000);
};

#endif // !_CAMERA_PERSPECTIVE_H_

