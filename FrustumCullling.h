/*
FrustumClass.h

Author: Zhuang Theerapong

*/
#pragma once
#ifndef _FRUSTUM_CULLING_H_
#define _FRUSTUM_CULLING_H_
#include "Camera.h"
#include <DirectXMath.h>
class FrustumCulling {
public:
	enum CULLING_TYPE {
		NONE,
		RECTANGLE,
		SPHERE,
		CUBE
	};
	struct Culling {
		float xCenter;
		float yCenter;
		float zCenter;
		float xOffset;
		float yOffset;
		float zOffset;
		virtual ~Culling() {};
	};
	struct Regtangle :Culling {
		float xSize;
		float ySize;
		float zSize;
	};
	struct Sphere :Culling {
		float radius;
	};
	struct Cube :Culling {
		float radius;
	};
	bool CheckPoint(float x, float y, float z);
	bool CheckCube(float xCenter, float yCenter, float zCenter, float radius);
	bool CheckSphere(float xCenter, float yCenter, float zCenter, float radius);
	bool CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);
	void ConstructFrustum(float screenDepth, DirectX::XMMATRIX projectionMatrix, DirectX::XMMATRIX viewMatrix);
	DirectX::XMVECTOR GetPlane(int i);

private:
	DirectX::XMVECTOR m_planes[6];

};
#endif