#include "EditorCamera.h"

using namespace DirectX;

EditorCamera::EditorCamera(int width, int height) 
	: PerspectiveCamera(width,height,45,XMFLOAT3(0,1,0), 
		XMFLOAT3(0, 1.7f, -1), XMFLOAT3(0, 1.7f, 1), 0.1f, 1000.0f),
	m_yaw(0),
	m_pitch(0),
	m_turnSpeed(1),
	m_mouSensitiveX(0.0025f),
	m_mouSensitiveY(0.0025f),
	m_moveSpeed(0.4f)
{
	
	m_maxRadiusPitch = XMConvertToRadians(85.f);
	m_minRadiusPitch = XMConvertToRadians(-85.f);
}
EditorCamera::~EditorCamera()
{

}
void EditorCamera::Update(KMInputManager* key)
{
	XMFLOAT3 move(0, 0, 0);
	// Calculate the move vector in camera space.
	if (key->GetKeyboardPress(DIK_A))
		move.x -= m_moveSpeed;
	if (key->GetKeyboardPress(DIK_D))
		move.x += m_moveSpeed;
	if (key->GetKeyboardPress(DIK_W))
		move.z += m_moveSpeed;
	if (key->GetKeyboardPress(DIK_S))
		move.z -= m_moveSpeed;
	//update camera's rotation



	XMFLOAT2 frameMouse(0, 0);

	if (key->GetMouseMiddleClick()) {
		int x, y;
		key->GetMouseFrameRate(x, y);
		frameMouse.x = (float)x;
		frameMouse.y = (float)y;
	}
	XMFLOAT2 prevMouse = m_mousePos;

	XMFLOAT2 newPos;
	newPos.x = prevMouse.x + frameMouse.x;
	newPos.y = prevMouse.y + frameMouse.y;

	XMFLOAT2 mouseDelta;
	mouseDelta.x = newPos.x - prevMouse.x;
	mouseDelta.y = newPos.y - prevMouse.y;


	m_yaw += m_mouSensitiveX * mouseDelta.x;

	m_pitch += m_mouSensitiveY * mouseDelta.y;

	if (m_pitch >= m_maxRadiusPitch) {
		m_pitch = m_maxRadiusPitch;
	}
	if (m_pitch <= m_minRadiusPitch) {
		m_pitch = m_minRadiusPitch;
	}
	m_mousePos = newPos;

	XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR DefaultRight = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
	XMVECTOR camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	//walk
	XMVECTOR camRight = XMVector3TransformCoord(DefaultRight, camRotationMatrix);
	XMVECTOR camForward = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	XMVECTOR camUp = XMVector3Cross(camForward, camRight);


	XMVECTOR velocity = move.x * camRight;
	velocity += move.z * camForward;
	m_camPos += velocity;
	camTarget = m_camPos + camTarget;

	XMFLOAT3 fCamTarget;

	XMStoreFloat3(&fCamTarget, camTarget);

	m_view = XMMatrixLookAtLH(m_camPos, camTarget, camUp);
}