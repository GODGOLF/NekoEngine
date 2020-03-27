
#ifndef _LIGHT_MANAGER_H_
#define _LIGHT_MANAGER_H_
#include "LightManagementInF.h"
#include <vector>
class LightManager : public LightManagementInF
{
public:
	LightManager();
	virtual ~LightManager();
	virtual bool AddLight(LightObjInF* pLightObj) override;
	virtual bool RemoveLight(LightObjInF* pLightObj) override;
	virtual void Destroy() override;
	std::vector<LightObjInF*>* GetLightArray();
private:
	std::vector<LightObjInF*> m_lights;
};


#endif
