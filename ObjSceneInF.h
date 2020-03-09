#ifndef _OBJ_SCENE_INF_H_
#define _OBJ_SCENE_INF_H_ 
#include <Windows.h>
class ModelInF
{
public:
	ModelInF() {};
	virtual ~ModelInF() {};
};

class ObjScene
{
public:
	ObjScene() {};
	virtual ~ObjScene() {};
	virtual bool AddObj(char* file, ModelInF* pModelObj)=0;
	virtual bool AddObj(ModelInF* pModelObj) = 0;
	virtual bool RemoveObj(ModelInF* pModelObj) = 0;
	virtual void Destroy() = 0;
};

#endif // !_OBJ_SCENE_INF_H_

