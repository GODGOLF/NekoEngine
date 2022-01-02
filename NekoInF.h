#ifndef _NEKO_INF_H_
#define _NEKO_INF_H_
#include "ObjSceneInF.h"
#include "LightManagementInF.h"
#include "Camera.h"
class NekoInF
{
public:
	virtual ~NekoInF() {};
	static NekoInF* CreateEngineManager(HWND* hwnd,
		unsigned int width,
		unsigned int height,
		ObjScene** pObjScene,
		LightManagementInF** pLightManager);
	virtual void OnRender(Camera* pCamera) =0;
	virtual void OnDestroy() =0;
};

#endif // !_NEKO_INF_H_

