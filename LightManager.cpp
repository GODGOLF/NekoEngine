#include "LightManager.h"
#include "FunctionHelper.h"

LightManager::LightManager()
{

}
LightManager::~LightManager()
{
	Destroy();
}
bool LightManager::AddLight(LightObjInF* pLightObj)
{
	m_lights.push_back(pLightObj);
	return true;
}
bool LightManager::RemoveLight(LightObjInF* pLightObj)
{
	return true;
}
void LightManager::Destroy()
{
	for (unsigned int i=0;i<m_lights.size();i++)
	{
		//delete m_lights[i];
		m_lights[i] = NULL;
	}
	m_lights.clear();
}
std::vector<LightObjInF*>* LightManager::GetLightArray()
{
	return &m_lights;
}