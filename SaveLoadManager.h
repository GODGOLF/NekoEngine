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
#include <string>
struct SaveLoadData
{
	enum OBJ_TYPE
	{
		PBR_OBJ,
		LIGHT
	};

	char* name;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 quaternion;
	DirectX::XMFLOAT3 scale;
	struct PBRDesc
	{
		
		
		float maxTessDistance;
		float minTessDistance;
		float maxTessFactor;
		float minTessFactor;
		float heightScale;
		std::string fileName;
		bool alphaTranparent;
	};
	PBRDesc pbrDesc;
	struct LightDesc
	{
		enum LightType
		{
			DIRECTION_LIGHT,
			POINT_LIGHT,
			SPOT_LIGHT
		};
		LightType lightType;
		DirectX::XMFLOAT4 Color;
		float Intensity;
		bool DisplayShadow;
		DirectX::XMFLOAT3 Direction;
		float OuterAngle;
		float InnerAngle;
		float Radius;
	};
	LightDesc lightDesc;
	OBJ_TYPE type;
};
class SaveLoadManager {
public:
	SaveLoadManager();
	~SaveLoadManager();
	/*
	* load data from file
	* para[0] object's data that want to save
	* para[1] size of data
	* para[2] file
	*
	*/
	static void Save(SaveLoadData* pObjects,int size, std::string outputFile);
	/*
	* allocate memory and load data from file
	* para[0] destination of object's data
	* para[1] file
	* para[2] size of data
	* 
	* after used, we MUST delete data
	*/
	static void Load(SaveLoadData** destObjects, std::string inputFile, int* destSize);
};

#endif // !SAVE_LOAD_MAP_MANAGER

