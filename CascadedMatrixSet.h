/*
CascadeMatrixSet.h

Author: Zhuang Theerapong

*/
#ifndef _CASCADED_MATRIX_SET_H_
#define _CASCADED_MATRIX_SET_H_
#include "Camera.h"
class CascadedMatrixSet {
public:
	CascadedMatrixSet();
	~CascadedMatrixSet();
	void Init(int iShadowMapSize);
	void Update(const DirectX::XMFLOAT3 &vDir, Camera* camera);
	// Change the antiflicker state
	void SetAntiFlicker(bool bIsOn) { m_bAntiFlickerOn = bIsOn; }

	const DirectX::XMMATRIX* GetWorldToShadowSpace() const { return &m_WorldToShadowSpace; }
	const DirectX::XMMATRIX* GetWorldToCascadeProj(int i) const { return &m_arrWorldToCascadeProj[i]; }
	const DirectX::XMFLOAT4 GetToCascadeOffsetX() const;
	const DirectX::XMFLOAT4 GetToCascadeOffsetY() const;
	const DirectX::XMFLOAT4 GetToCascadeScale() const;
	const float GetTotalRange() const;

	static const int m_iTotalCascades = 3;
private:
	// Extract the frustum corners for the given near and far values
	void ExtractFrustumPoints(float fNear, float fFar, DirectX::XMFLOAT3* arrFrustumCorners, Camera* camera);

	// Extract the frustum bounding sphere for the given near and far values
	void ExtractFrustumBoundSphere(float fNear, float fFar, DirectX::XMFLOAT3& vBoundCenter, float& fBoundRadius, Camera* camera);

	// Test if a cascade needs an update
	bool CascadeNeedsUpdate(const DirectX::XMMATRIX& mShadowView, int iCascadeIdx, const DirectX::XMFLOAT3& newCenter, DirectX::XMFLOAT3& vOffset);
	bool m_bAntiFlickerOn;
	int m_iShadowMapSize;
	float m_fCascadeTotalRange;
	float m_arrCascadeRanges[4];

	DirectX::XMFLOAT3 m_vShadowBoundCenter;
	float m_fShadowBoundRadius;
	DirectX::XMFLOAT3 m_arrCascadeBoundCenter[m_iTotalCascades];
	float m_arrCascadeBoundRadius[m_iTotalCascades];

	DirectX::XMMATRIX m_WorldToShadowSpace;
	DirectX::XMMATRIX m_arrWorldToCascadeProj[m_iTotalCascades];

	float m_vToCascadeOffsetX[4];
	float m_vToCascadeOffsetY[4];
	float m_vToCascadeScale[4];
};
#endif