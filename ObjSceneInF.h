#ifndef _OBJ_SCENE_INF_H_
#define _OBJ_SCENE_INF_H_ 
#include <Windows.h>
#include "ModelInF.h"
struct ObjDesc
{
	enum OBJ_TYPE
	{
		MODEL_OBJECT,
		TERRAIN_OBJECT,
		OCEAN_OBJECT
	};
	OBJ_TYPE type;
	struct ModelDesc
	{
		const char* file;
	};
	ModelDesc modelDesc;
	struct TerrainDesc
	{
		const char* file;
		const char* diffuseTextureFile;
		int arraySize;
		int ceilSize;
		int dimension;
		DirectX::XMFLOAT4 diffuseColor;
	};
	TerrainDesc terrainDesc;
	struct OceanDesc
	{
		const char* name;
		int arraySize;
		int ceilSize;
		int dimension;
		DirectX::XMFLOAT4 diffuseColor;
		const char* normalTexture;
	};
	OceanDesc oceanDesc;
};

class ObjScene
{
public:
	virtual ~ObjScene() {};
	virtual bool AddObj(ModelInF** pModelObj, ObjDesc desc)=0;
	virtual bool AddObj(ModelInF* pModelObj) = 0;
	virtual bool RemoveObj(ModelInF* pModelObj) = 0;
	virtual void Destroy() = 0;
};

#endif // !_OBJ_SCENE_INF_H_

