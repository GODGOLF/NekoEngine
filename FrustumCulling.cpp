#include "FrustumCullling.h"
using namespace DirectX;

void FrustumCulling::ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix)
{
	float zMinimum, r;
	XMMATRIX matrix;

	XMFLOAT4X4 fProjection;

	XMMATRIX tempProjectmatrix;

	XMStoreFloat4x4(&fProjection, projectionMatrix);

	// Calculate the minimum Z distance in the frustum.
	zMinimum = -fProjection._43 / fProjection._33;
	r = screenDepth / (screenDepth - zMinimum);
	fProjection._33 = r;
	fProjection._43 = -r * zMinimum;

	tempProjectmatrix = XMLoadFloat4x4(&fProjection);

	// Create the frustum matrix from the view matrix and updated projection matrix.
	matrix = XMMatrixMultiply(viewMatrix, tempProjectmatrix);


	XMFLOAT4X4 fMatrix;

	XMStoreFloat4x4(&fMatrix, matrix);

	XMFLOAT4 mP1;
	// Calculate near plane of frustum.
	mP1.x = fMatrix._14 + fMatrix._13;
	mP1.y = fMatrix._24 + fMatrix._23;
	mP1.z = fMatrix._34 + fMatrix._33;
	mP1.w = fMatrix._44 + fMatrix._43;
	m_planes[0] = XMLoadFloat4(&mP1);
	m_planes[0] = XMVector4Normalize(m_planes[0]);


	// Calculate far plane of frustum.
	XMFLOAT4 mP2;
	mP2.x = fMatrix._14 - fMatrix._13;
	mP2.y = fMatrix._24 - fMatrix._23;
	mP2.z = fMatrix._34 - fMatrix._33;
	mP2.w = fMatrix._44 - fMatrix._43;
	m_planes[1] = XMLoadFloat4(&mP2);
	m_planes[1] = XMPlaneNormalize(m_planes[1]);


	// Calculate left plane of frustum.
	XMFLOAT4 mP3;
	mP3.x = fMatrix._14 + fMatrix._11;
	mP3.y = fMatrix._24 + fMatrix._21;
	mP3.z = fMatrix._34 + fMatrix._31;
	mP3.w = fMatrix._44 + fMatrix._41;
	m_planes[2] = XMLoadFloat4(&mP3);
	m_planes[2] = XMPlaneNormalize(m_planes[2]);

	// Calculate right plane of frustum.
	XMFLOAT4 mP4;
	mP4.x = fMatrix._14 - fMatrix._11;
	mP4.y = fMatrix._24 - fMatrix._21;
	mP4.z = fMatrix._34 - fMatrix._31;
	mP4.w = fMatrix._44 - fMatrix._41;
	m_planes[3] = XMLoadFloat4(&mP4);
	m_planes[3] = XMPlaneNormalize(m_planes[3]);

	// Calculate top plane of frustum.
	XMFLOAT4 mP5;
	mP5.x = fMatrix._14 - fMatrix._12;
	mP5.y = fMatrix._24 - fMatrix._22;
	mP5.z = fMatrix._34 - fMatrix._32;
	mP5.w = fMatrix._44 - fMatrix._42;
	m_planes[4] = XMLoadFloat4(&mP5);
	m_planes[4] = XMPlaneNormalize(m_planes[4]);

	// Calculate bottom plane of frustum.
	XMFLOAT4 mP6;
	mP6.x = fMatrix._14 + fMatrix._12;
	mP6.y = fMatrix._24 + fMatrix._22;
	mP6.z = fMatrix._34 + fMatrix._32;
	mP6.w = fMatrix._44 + fMatrix._42;
	m_planes[5] = XMLoadFloat4(&mP6);
	m_planes[5] = XMPlaneNormalize(m_planes[5]);

	return;
}

bool FrustumCulling::CheckPoint(float x, float y, float z) {
	XMVECTOR tempPoint;
	XMFLOAT3 point(x, y, z);
	tempPoint = XMLoadFloat3(&point);
	// Check if the point is inside all six planes of the view frustum.
	for (int i = 0; i<6; i++)
	{
		XMVECTOR result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMFLOAT3 result2;
		XMStoreFloat3(&result2, result);
		if (result2.x < 0.0f)
		{
			return false;
		}
	}

	return true;
}
bool FrustumCulling::CheckCube(float xCenter, float yCenter, float zCenter, float radius) {


	// Check if any one point of the cube is in the view frustum.
	for (int i = 0; i<6; i++)
	{
		XMVECTOR tempPoint;
		XMFLOAT3 position((xCenter - radius), (yCenter - radius), (zCenter - radius));
		tempPoint = XMLoadFloat3(&position);
		XMVECTOR result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMFLOAT3 fResult;
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter + radius), (yCenter - radius), (zCenter - radius));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter - radius), (yCenter + radius), (zCenter - radius));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter + radius), (yCenter + radius), (zCenter - radius));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);

		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter - radius), (yCenter - radius), (zCenter + radius));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter + radius), (yCenter - radius), (zCenter + radius));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter - radius), (yCenter + radius), (zCenter + radius));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);

		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter + radius), (yCenter + radius), (zCenter + radius));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}

		return false;
	}

	return true;

}
bool FrustumCulling::CheckSphere(float xCenter, float yCenter, float zCenter, float radius) {
	XMVECTOR tempPoint;
	XMFLOAT3 point(xCenter, yCenter, zCenter);
	tempPoint = XMLoadFloat3(&point);
	// Check if the radius of the sphere is inside the view frustum.
	for (int i = 0; i<6; i++)
	{
		XMVECTOR result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMFLOAT3 result2;
		XMStoreFloat3(&result2, result);
		if (result2.x < -radius)
		{

			return false;
		}

	}
	return true;
}
bool FrustumCulling::CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize) {



	// Check if any of the 6 planes of the rectangle are inside the view frustum.
	for (int i = 0; i<6; i++)
	{
		XMVECTOR tempPoint;
		XMFLOAT3 position((xCenter - xSize), (yCenter - ySize), (zCenter - zSize));
		tempPoint = XMLoadFloat3(&position);
		XMVECTOR result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMFLOAT3 fResult;
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter + xSize), (yCenter - ySize), (zCenter - zSize));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter - xSize), (yCenter + ySize), (zCenter - zSize));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter - xSize), (yCenter - ySize), (zCenter + zSize));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter + xSize), (yCenter + ySize), (zCenter - zSize));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter + xSize), (yCenter - ySize), (zCenter + zSize));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter - xSize), (yCenter + ySize), (zCenter + zSize));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}
		position = XMFLOAT3((xCenter + xSize), (yCenter + ySize), (zCenter + zSize));
		tempPoint = XMLoadFloat3(&position);
		result = XMPlaneDotCoord(m_planes[i], tempPoint);
		XMStoreFloat3(&fResult, result);
		if (fResult.x >= 0.0f)
		{
			continue;
		}

		return false;
	}

	return true;
}
XMVECTOR FrustumCulling::GetPlane(int i) {
	if (i>5) {
		return XMVECTOR();
	}
	return m_planes[i];
}
