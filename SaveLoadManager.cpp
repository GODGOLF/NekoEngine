#include "SaveLoadManager.h"
#include "JSON/json.hpp"
#include <iostream>
#define OBJECT_LIST			"OBJECT"
#define NAME				"NAME"
#define POS					"POS"
#define ROT					"ROT"
#define SCALE				"SCALE"
#define FILE_NAME			"FILE"
#define OBJECT_TYPE			"TYPE"
#define TESS_DIS_MAX		"TESS_DIS_MAX"
#define TESS_DIS_MIN		"TESS_DIS_MIN"
#define TESS_FACTOR_MAX		"TESS_FACTOR_MAX"
#define TESS_FACTOR_MIN		"TESS_FACTOR_MIN"
#define HIGHT_SCALE			"HIGHT_SCALE"
#define ALPHA				"ALPHA"

#define LIGHT_TYPE			"LIGHT_TYPE"
#define LIGHT_RADIUS		"LIGHT_RADIUS"
#define OUTER_ANGLE			"OUTER_ANGLE"
#define INNER_ANGLE			"INNER_ANGLE"
#define LIGHT_DIRECTION		"LIGHT_DIRECTION"
#define LIGHT_COLOR			"LIGHT_COLOR"
#define LIGHT_INTENSITY		"LIGHT_INTENSITY"
using json = nlohmann::json;
SaveLoadManager::SaveLoadManager()
{

}
SaveLoadManager::~SaveLoadManager()
{

}
void SaveLoadManager::Save(SaveLoadData* pObjects, int size, std::string inputFile)
{
	std::vector<json> root;
	root.resize(size);
	for (int i = 0; i < size; i++)
	{
		json j_info;
		j_info[NAME] = pObjects->name;
		j_info[OBJECT_TYPE] = (int)(pObjects->type);
		//position
		std::vector<float> tempPos;
		tempPos.resize(3);
		tempPos[0] = pObjects->position.x;
		tempPos[1] = pObjects->position.y;
		tempPos[2] = pObjects->position.z;
		j_info[POS] = tempPos;
		//quaterion
		std::vector<float> tempRot;
		tempRot.resize(4);
		tempRot[0] = pObjects->quaternion.x;
		tempRot[1] = pObjects->quaternion.y;
		tempRot[2] = pObjects->quaternion.z;
		tempRot[3] = pObjects->quaternion.w;
		j_info[ROT] = tempRot;
		//scale
		std::vector<float> tempScale;
		tempScale.resize(3);
		tempScale[0] = pObjects->scale.x;
		tempScale[1] = pObjects->scale.y;
		tempScale[2] = pObjects->scale.z;
		json j_scale(tempScale);
		j_info[SCALE] = j_scale;
		switch (pObjects->type)
		{
		case SaveLoadData::PBR_OBJ: 
		{
			j_info[TESS_DIS_MAX] = pObjects->pbrDesc.maxTessDistance;
			j_info[TESS_DIS_MIN] = pObjects->pbrDesc.minTessDistance;
			j_info[TESS_FACTOR_MAX] = pObjects->pbrDesc.maxTessFactor;
			j_info[TESS_FACTOR_MIN] = pObjects->pbrDesc.minTessFactor;
			j_info[HIGHT_SCALE] = pObjects->pbrDesc.heightScale;
			j_info[ALPHA] = pObjects->pbrDesc.alphaTranparent;
			j_info[FILE_NAME] = pObjects->pbrDesc.fileName;
			
		}
		break;
		case SaveLoadData::LIGHT:
		{
			j_info[LIGHT_TYPE] = pObjects->lightDesc.lightType;
			j_info[LIGHT_RADIUS] = pObjects->lightDesc.Radius;
			j_info[OUTER_ANGLE] = pObjects->lightDesc.OuterAngle;
			j_info[INNER_ANGLE] = pObjects->lightDesc.InnerAngle;
			j_info[LIGHT_INTENSITY] = pObjects->lightDesc.Intensity;

			std::vector<float> tempDirection;
			tempDirection.resize(3);
			tempDirection[0] = pObjects->lightDesc.Direction.x;
			tempDirection[1] = pObjects->lightDesc.Direction.y;
			tempDirection[2] = pObjects->lightDesc.Direction.z;
			j_info[LIGHT_DIRECTION] = tempDirection;

			std::vector<float> tempColor;
			tempColor.resize(4);
			tempColor[0] = pObjects->lightDesc.Color.x;
			tempColor[1] = pObjects->lightDesc.Color.y;
			tempColor[2] = pObjects->lightDesc.Color.z;
			tempColor[3] = pObjects->lightDesc.Color.w;
			j_info[LIGHT_COLOR] = tempColor;
		}
		break;
		default:
			break;
		}
		pObjects++;
		root[i] = j_info;
	}
	json data;
	data[OBJECT_LIST] = root;
	std::ofstream dataStream(inputFile, std::ios::binary);

	dataStream.write(data.dump().c_str(), data.dump().size());

	dataStream.close();
}
void SaveLoadManager::Load(SaveLoadData** destObjects, std::string inputFile, int* destSize)
{
	std::ifstream input(inputFile, std::ios::in | std::ios::binary);
	json j_data;
	input >> j_data;
	SaveLoadData* object = *destObjects;
	for (auto& i : j_data.items())
	{
		int index =0;
		if (i.key() == OBJECT_LIST) {
			//allocate data
			object = new SaveLoadData[i.value().size()];
			for (auto& j : i.value())
			{
				json pos = j[POS];
				float fPos[3] = { 0.f,0.f,0.f };
				int iPos = 0;
				for (auto& k : pos.items()) {
					fPos[iPos] = k.value();
					iPos++;
				}
				object[index].position = DirectX::XMFLOAT3(&fPos[0]);
				json rot = j[ROT];
				float fRot[4] = { 0.f,0.f,0.f,0.f };
				int iRot = 0;
				for (auto& k : rot.items()) {
					fRot[iRot] = k.value();
					iRot++;
				}
				object[index].quaternion = DirectX::XMFLOAT4(&fRot[0]);

				json scale = j[SCALE];
				int iScale = 0;
				float fScale[3] = { 0.f,0.f,0.f };
				for (auto& k : scale.items()) {
					fScale[iScale] = k.value();
					iScale++;
				}
				object[index].scale = DirectX::XMFLOAT3(&fScale[0]);
				if (j[OBJECT_TYPE] == SaveLoadData::OBJ_TYPE::PBR_OBJ)
				{
					object[index].type = SaveLoadData::OBJ_TYPE::PBR_OBJ;
					std::string tempFileName = j[FILE_NAME];
					object[index].pbrDesc.fileName = tempFileName;
					object[index].pbrDesc.alphaTranparent = j[ALPHA];
					object[index].pbrDesc.maxTessDistance = j[TESS_DIS_MAX];
					object[index].pbrDesc.minTessDistance = j[TESS_DIS_MIN];
					object[index].pbrDesc.maxTessFactor = j[TESS_FACTOR_MAX];
					object[index].pbrDesc.minTessFactor = j[TESS_FACTOR_MIN];
					object[index].pbrDesc.heightScale = j[HIGHT_SCALE];
				}
				else if(j[OBJECT_TYPE] == SaveLoadData::OBJ_TYPE::LIGHT)
				{
					json dir = j[LIGHT_DIRECTION];
					float fDir[3] = { 0.f,0.f,0.f };
					int iDir = 0;
					for (auto& k : dir.items()) {
						fPos[iDir] = k.value();
						iDir++;
					}
					object[index].lightDesc.Direction = DirectX::XMFLOAT3(&fDir[0]);
					json color = j[LIGHT_COLOR];
					float fColor[4] = { 0.f,0.f,0.f,0.f };
					int iColor = 0;
					for (auto& k : color.items()) {
						fColor[iColor] = k.value();
						iColor++;
					}
					object[index].lightDesc.Color = DirectX::XMFLOAT4(&fColor[0]);
					object[index].lightDesc.Radius = j[LIGHT_RADIUS];
					object[index].lightDesc.InnerAngle = j[INNER_ANGLE];
					object[index].lightDesc.OuterAngle = j[OUTER_ANGLE];
					object[index].lightDesc.lightType = (SaveLoadData::LightDesc::LightType)j[LIGHT_TYPE];
					object[index].lightDesc.Intensity = j[LIGHT_INTENSITY];
				}
			}
		}
	}
}
