#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <DirectXMath.h>
class Camera
{
public:
	Camera();
	virtual ~Camera();

	DirectX::XMMATRIX GetView()			const;
	DirectX::XMMATRIX GetProjection()	const;
	DirectX::XMVECTOR GetPosition()		const;
	float GetNearValue()				const;
	float GetFarValue()					const;
	DirectX::XMVECTOR GetWorldRight()	const;
	DirectX::XMVECTOR GetWorldUp()		const;
	DirectX::XMVECTOR GetWorldAhead()	const;

protected:
	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_projection;
	DirectX::XMVECTOR m_camPos;
	float m_nearValue;
	float m_farValue;
	

};
#endif // !_CAMERA_H_

