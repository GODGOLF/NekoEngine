#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <DirectXMath.h>
class Camera
{
public:
	Camera();
	~Camera();

	DirectX::XMMATRIX GetView();
	DirectX::XMMATRIX GetProjection();
	DirectX::XMVECTOR GetPosition();
	float GetNearValue();
	float GetFarValue();
	DirectX::XMVECTOR GetWorldRight();
	DirectX::XMVECTOR GetWorldUp();
	DirectX::XMVECTOR GetWorldAhead();
	float GetFOV();
	float GetAspectRadio();

protected:
	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_projection;
	DirectX::XMVECTOR m_camPos;
	float m_nearValue;
	float m_farValue;
	float m_aspectRadio;
	float m_FOV;

};
#endif // !_CAMERA_H_

