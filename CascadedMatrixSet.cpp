#include "CascadedMatrixSet.h"
#include "PerspectiveCamera.h"
#include "FunctionHelper.h"
#include <algorithm>

#define ZOOM_VALUE	10.f

using namespace DirectX;

CascadedMatrixSet::CascadedMatrixSet() : m_bAntiFlickerOn(true), m_fCascadeTotalRange(100.0f) 
{

}
CascadedMatrixSet::~CascadedMatrixSet() 
{

}

void CascadedMatrixSet::Init(int iShadowMapSize) {

	float length = m_fCascadeTotalRange / m_iTotalCascades;
	m_iShadowMapSize = iShadowMapSize;
	for (int i = 0; i < 4; i++)
	{
		m_arrCascadeRanges[i] = length * i;
	}
	m_fCascadeTotalRange = m_arrCascadeRanges[3];
	for (int i = 0; i < m_iTotalCascades; i++)
	{
		m_arrCascadeBoundCenter[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_arrCascadeBoundRadius[i] = 0.0f;
	}
}
void CascadedMatrixSet::Update(const XMFLOAT3 &fDir, Camera* camera)
{
	//Calculate view Matrix
	XMVECTOR vWorldCenter = camera->GetPosition() +camera->GetWorldAhead() * m_fCascadeTotalRange *0.5f;
	XMVECTOR vPos = vWorldCenter;
	XMVECTOR vLookAt = vWorldCenter + (XMLoadFloat3(&fDir)) * XMVectorSet(m_fCascadeTotalRange, m_fCascadeTotalRange, m_fCascadeTotalRange, 0);
	XMVECTOR vUP;
	XMFLOAT3 fRight = XMFLOAT3(1, 0, 0);
	XMVECTOR vRight = XMLoadFloat3(&fRight);
	XMVECTOR vDir = XMLoadFloat3(&fDir);
	vUP = XMVector3Cross(vDir, vRight);
	vUP = XMVector3Normalize(vUP);
	XMMATRIX mShadowView = XMMatrixLookAtLH(vPos, vLookAt, vUP);

	// Get the bounds for the shadow space
	ExtractFrustumBoundSphere(m_arrCascadeRanges[0], m_arrCascadeRanges[3], m_vShadowBoundCenter, m_fShadowBoundRadius, camera);
	
	//calculate shadow space
	XMMATRIX mShadowProj = XMMatrixOrthographicLH(m_fShadowBoundRadius, m_fShadowBoundRadius, -m_fShadowBoundRadius, m_fShadowBoundRadius);
	// The combined transformation from world to shadow space
	m_WorldToShadowSpace = mShadowView * mShadowProj;

	XMMATRIX mShadowViewInv = XMMatrixTranspose(mShadowView);

	
	for (int iCascadeIdx = 0; iCascadeIdx < m_iTotalCascades; iCascadeIdx++)
	{
		XMMATRIX cascadeTrans;
		XMMATRIX cascadeScale;
		if (m_bAntiFlickerOn)
		{
			XMFLOAT3 vNewCenter;
			float fRadius =0;
			ExtractFrustumBoundSphere(m_arrCascadeRanges[iCascadeIdx], m_arrCascadeRanges[iCascadeIdx + 1], vNewCenter, fRadius, camera);
			m_arrCascadeBoundRadius[iCascadeIdx] = std::max(m_arrCascadeBoundRadius[iCascadeIdx], fRadius); // Expend the radius to compensate for numerical errors
																								   // Only update the cascade bounds if it moved at least a full pixel unit																			   // This makes the transformation invariant to translation
			XMFLOAT3 fOffset = XMFLOAT3(0,0,0);
			if (CascadeNeedsUpdate(mShadowView, iCascadeIdx, vNewCenter, fOffset))
			{
				// To avoid flickering we need to move the bound center in full units
				XMVECTOR vOffset = XMLoadFloat3(&fOffset);
				XMVECTOR vOffsetOut = XMVector3TransformNormal(vOffset, mShadowViewInv);
				XMVECTOR vBoundCenter = XMLoadFloat3(&m_arrCascadeBoundCenter[iCascadeIdx]);
				vBoundCenter += vOffsetOut;
				XMStoreFloat3(&m_arrCascadeBoundCenter[iCascadeIdx], vBoundCenter);
			}
			// Get the cascade center in shadow space
			XMVECTOR vCascadeCenterShadowSpace;
			XMVECTOR vBoundCenter = XMLoadFloat3(&m_arrCascadeBoundCenter[iCascadeIdx]);
			vCascadeCenterShadowSpace = XMVector3TransformCoord(vBoundCenter, m_WorldToShadowSpace);

			XMFLOAT3 fCascadeCenterShadowSpace;
			XMStoreFloat3(&fCascadeCenterShadowSpace, vCascadeCenterShadowSpace);
			// Update the translation from shadow to cascade space
			m_vToCascadeOffsetX[iCascadeIdx] = -fCascadeCenterShadowSpace.x;
			m_vToCascadeOffsetY[iCascadeIdx] = -fCascadeCenterShadowSpace.y;

			cascadeTrans = XMMatrixTranslation(m_vToCascadeOffsetX[iCascadeIdx], m_vToCascadeOffsetY[iCascadeIdx], 0.0f);

			// Update the scale from shadow to cascade space
			m_vToCascadeScale[iCascadeIdx] = m_fShadowBoundRadius / m_arrCascadeBoundRadius[iCascadeIdx];

			cascadeScale = XMMatrixScaling(m_vToCascadeScale[iCascadeIdx], m_vToCascadeScale[iCascadeIdx], 1.0f);
		}
		else {
			// Since we don't care about flickering we can make the cascade fit tightly around the frustum
			// Extract the bounding box
			XMFLOAT3 arrFrustumPoints[8];
			ExtractFrustumPoints(m_arrCascadeRanges[iCascadeIdx], m_arrCascadeRanges[iCascadeIdx + 1], arrFrustumPoints, camera);
			// Transform to shadow space and extract the minimum andn maximum
			XMFLOAT3 fMin = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
			XMFLOAT3 fMax = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			for (int i = 0; i < 8; i++)
			{
				XMVECTOR vPointInShadowSpace = XMVector3TransformCoord(XMLoadFloat3(&arrFrustumPoints[i]), m_WorldToShadowSpace);

				XMFLOAT3 fPointInShadowSpace;

				XMStoreFloat3(&fPointInShadowSpace, vPointInShadowSpace);
				for (int j = 0; j < 3; j++)
				{
					if ((&fMin.x)[j] >(&fPointInShadowSpace.x)[j])
						(&fMin.x)[j] = (&fPointInShadowSpace.x)[j];
					if ((&fMax.x)[j] < (&fPointInShadowSpace.x)[j])
						(&fMax.x)[j] = (&fPointInShadowSpace.x)[j];
				}
			}
			XMVECTOR vMin = XMLoadFloat3(&fMin);
			XMVECTOR vMax = XMLoadFloat3(&fMax);
			XMVECTOR vCascadeCenterShadowSpace = 0.5f * (vMin + vMax);
			XMFLOAT3 fCascadeCenterShadowSpace;
			XMStoreFloat3(&fCascadeCenterShadowSpace, vCascadeCenterShadowSpace);

			// Update the translation from shadow to cascade space
			m_vToCascadeOffsetX[iCascadeIdx] = -fCascadeCenterShadowSpace.x;
			m_vToCascadeOffsetY[iCascadeIdx] = -fCascadeCenterShadowSpace.y;
			cascadeTrans = XMMatrixTranslation(m_vToCascadeOffsetX[iCascadeIdx], m_vToCascadeOffsetY[iCascadeIdx], 0.0f);

			// Update the scale from shadow to cascade space
			m_vToCascadeScale[iCascadeIdx] = 2.0f / std::max(fMax.x - fMin.x, fMax.y - fMin.y);
			cascadeScale = XMMatrixScaling(m_vToCascadeScale[iCascadeIdx], m_vToCascadeScale[iCascadeIdx], 1.0f);

		}
		// Combine the matrices to get the transformation from world to cascade space
		m_arrWorldToCascadeProj[iCascadeIdx] = m_WorldToShadowSpace * cascadeTrans * cascadeScale;
	}
	// Set the values for the unused slots to someplace outside the shadow space
	for (int i = m_iTotalCascades; i < 4; i++)
	{
		m_vToCascadeOffsetX[i] = 250.0f;
		m_vToCascadeOffsetY[i] = 250.0f;
		m_vToCascadeScale[i] = 0.1f;
	}


}
void CascadedMatrixSet::ExtractFrustumBoundSphere(float fNear, float fFar, DirectX::XMFLOAT3& vBoundCenter, float& fBoundRadius, Camera* camera)
{
	if (!DirectXHelper::instanceof<PerspectiveCamera>(camera))
	{
		return;
	}

	XMMATRIX projectionMatrix = camera->GetProjection();

	PerspectiveCamera* pCamera = (PerspectiveCamera*)camera;

	const XMVECTOR camPos = camera->GetPosition();
	const XMVECTOR camRight = camera->GetWorldRight();
	const XMVECTOR camUp = camera->GetWorldUp();
	const XMVECTOR camForward = camera->GetWorldAhead();
	// Calculate the tangent values (this can be cached)
	const float fTanFOVX = tanf(pCamera->GetAspectRadio() * pCamera->GetFOV());
	const float fTanFOVY = tanf(pCamera->GetFOV());

	// The center of the sphere is in the center of the frustum
	XMVECTOR vBoundC = camPos + camForward * (fNear + 0.5f * (fNear + fFar));
	XMStoreFloat3(&vBoundCenter, vBoundC);

	const XMVECTOR vBoundSpan = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar - vBoundC;

	const XMVECTOR lBoundSpan = XMVector3Length(vBoundSpan);

	XMFLOAT3 fBoundSpan;
	XMStoreFloat3(&fBoundSpan, lBoundSpan);
	fBoundRadius = fBoundSpan.x;
	
}
void CascadedMatrixSet::ExtractFrustumPoints(float fNear, float fFar, DirectX::XMFLOAT3* arrFrustumCorners, Camera* camera)
{
	if (!DirectXHelper::instanceof<PerspectiveCamera>(camera))
	{
		return;
	}
	const XMVECTOR camPos = camera->GetPosition();
	const XMVECTOR camRight = camera->GetWorldRight();
	const XMVECTOR camUp = camera->GetWorldUp();
	const XMVECTOR camForward = camera->GetWorldAhead();

	PerspectiveCamera* pCamera = (PerspectiveCamera*)camera;
	// Calculate the tangent values (this can be cached
	const float fTanFOVX = tanf(pCamera->GetAspectRadio() * pCamera->GetFOV());
	const float fTanFOVY = tanf(pCamera->GetAspectRadio());


	// Calculate the points on the near plane
	XMVECTOR near1 = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fNear;
	XMStoreFloat3(&arrFrustumCorners[0], near1);
	XMVECTOR near2 = camPos + (camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fNear;
	XMStoreFloat3(&arrFrustumCorners[1], near2);
	XMVECTOR near3 = camPos + (camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fNear;
	XMStoreFloat3(&arrFrustumCorners[2], near3);
	XMVECTOR near4 = camPos + (-camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fNear;
	XMStoreFloat3(&arrFrustumCorners[3], near4);

	// Calculate the points on the far plane
	XMVECTOR far1 = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar;
	XMStoreFloat3(&arrFrustumCorners[4], far1);
	XMVECTOR far2 = camPos + (camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar;
	XMStoreFloat3(&arrFrustumCorners[5], far2);
	XMVECTOR far3 = camPos + (camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fFar;
	XMStoreFloat3(&arrFrustumCorners[6], far3);
	XMVECTOR far4 = camPos + (-camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fFar;
	XMStoreFloat3(&arrFrustumCorners[7], far4);
	

}
const XMFLOAT4 CascadedMatrixSet::GetToCascadeOffsetX() const
{
	XMFLOAT4 offsetX = XMFLOAT4(m_vToCascadeOffsetX[0],
		m_vToCascadeOffsetX[1], m_vToCascadeOffsetX[2],
		m_vToCascadeOffsetX[3]);
	return offsetX;
}
const XMFLOAT4 CascadedMatrixSet::GetToCascadeOffsetY() const
{
	XMFLOAT4 offsetY = XMFLOAT4(m_vToCascadeOffsetY[0],
		m_vToCascadeOffsetY[1], m_vToCascadeOffsetY[2],
		m_vToCascadeOffsetY[3]);
	return offsetY;
}
const XMFLOAT4 CascadedMatrixSet::GetToCascadeScale()const
{
	XMFLOAT4 scale = XMFLOAT4(m_vToCascadeScale[0],
		m_vToCascadeScale[1], m_vToCascadeScale[2],
		m_vToCascadeScale[3]);
	return scale;
}

// Test if a cascade needs an update
bool CascadedMatrixSet::CascadeNeedsUpdate(const XMMATRIX& mShadowView, int iCascadeIdx, const XMFLOAT3& newCenter, XMFLOAT3& vOffset) {
	// Find the offset between the new and old bound ceter
	XMVECTOR vOldCenterInCascade = XMVector3TransformCoord(XMLoadFloat3(&m_arrCascadeBoundCenter[iCascadeIdx]), mShadowView);
	XMVECTOR vNewCenterInCascade = XMVector3TransformCoord(XMLoadFloat3(&newCenter), mShadowView);
	XMVECTOR vCenterDiff = vNewCenterInCascade - vOldCenterInCascade;
	// Find the pixel size based on the diameters and map pixel size
	float fPixelSize = (float)m_iShadowMapSize / (2.0f * m_arrCascadeBoundRadius[iCascadeIdx]);

	XMFLOAT3 fCenterDiff;
	XMStoreFloat3(&fCenterDiff, vCenterDiff);
	float fPixelOffX = fCenterDiff.x * fPixelSize;
	float fPixelOffY = fCenterDiff.y * fPixelSize;

	// Check if the center moved at least half a pixel unit
	bool bNeedUpdate = (abs(fPixelOffX)) > 0.5f || (abs(fPixelOffY) > 0.5f);
	if (bNeedUpdate)
	{
		// Round to the 
		vOffset.x = floorf(0.5f + fPixelOffX) / fPixelSize;
		vOffset.y = floorf(0.5f + fPixelOffY) / fPixelSize;
		vOffset.z = fCenterDiff.z;
	}

	return bNeedUpdate;
}
const float CascadedMatrixSet::GetTotalRange() const
{
	return m_fCascadeTotalRange;
}