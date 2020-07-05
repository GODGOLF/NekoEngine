/*
SaveLoadMapManager.h

Author: Zhuang Theerapong

*/
#ifndef SAVE_LOAD_MANAGER
#define SAVE_LOAD_MANAGER
#include <iostream>
#include <fstream>
#include <vector>
#include <DirectXMath.h>

struct SaveLoadData
{
	enum OBJ_TYPE
	{
		PBR_OBJ,
		SKY_BOX
	};
	struct PBRDesc
	{
		char* name;
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 quaternion;
		DirectX::XMFLOAT3 scale;
		float maxTessDistance;
		float minTessDistance;
		float maxTessFactor;
		float minTessFactor;
		float heightScale;
		char* fileName;
		bool alphaTranparent;
	};
	PBRDesc pbrDesc;
	struct SkyBoxDesc
	{
		char* fileName;
	};
};
class SaveLoadManager {
public:
	SaveLoadManager();
	~SaveLoadManager();
	static void Save(SaveLoadData* objects,int size);
	static void Load(SaveLoadData &objects);
};

#endif // !SAVE_LOAD_MAP_MANAGER

