#ifndef _OCEAN_OBJ_H_
#define _OCEAN_OBJ_H_

#include "PlaneObj.h"

class OceanObj : public PlaneObj
{
public:
	OceanObj() : m_time(0.f){};
	float m_time;
	struct WaveInfo
	{
		DirectX::XMFLOAT2 direction = DirectX::XMFLOAT2(1.f, 1.f);
		float waveLength = 10.f;
		float steepness = 0.5f;
	};
	WaveInfo m_waveInfo[3];
private:

};
#endif // !_TERRAIN_OBJ_H_

