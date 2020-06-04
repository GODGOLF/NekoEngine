#include "DXInclude.h"
#include "ObjManager.h"
#include "D3D11Model.h"
#include "AnimeObj.h"
#include "TerrainObj.h"
#include "D3D11TerrainModel.h"
#include "D3D11OceanModel.h"
#include "OceanObj.h"
ObjManager::ObjManager() :p_device(NULL),p_fbxManager(NULL)
{
	
}
ObjManager::~ObjManager()
{
	Destroy();
}
bool ObjManager::AddObj(ModelInF** pModelObj,ObjDesc pDesc)
{
	switch (pDesc.type)
	{
	case ObjDesc::OBJ_TYPE::MODEL_OBJECT:
	{
		D3DModelInF* model = NULL;
		if (m_modelObjectList.find(pDesc.modelDesc.file) == m_modelObjectList.end())
		{
			model = new D3D11Model();
			D3D11ModelParameterInitial* parameter = new D3D11ModelParameterInitial();
			parameter->pDevice = (D3D11Class*)p_device;
			parameter->pFbxManager = p_fbxManager;
			HRESULT hr = model->Initial((char*)pDesc.modelDesc.file, parameter);
			delete parameter;
			parameter = NULL;
			if (FAILED(hr))
			{
				delete model;
				return false;
			}
			//add obj in the map list
			m_modelObjectList[pDesc.modelDesc.file] = model;
		}
		else
		{
			model = m_modelObjectList[pDesc.modelDesc.file];
		}
		FBXLoader* modelData = ((D3D11Model*)model)->GetModelData();
		if (modelData->haveAnimation)
		{
			*pModelObj = new AnimeObj();
		}
		else
		{
			*pModelObj = new ModelInF();
		}
		
		(*pModelObj)->m_modelIndex = string(pDesc.modelDesc.file);
		//extra data from Model obj such as animation info
		(*pModelObj)->haveAnimation = modelData->haveAnimation;
		std::vector<AnimationStack>* animStack = modelData->GetAnimationStacks();
		for (unsigned int i = 0; i < animStack->size(); i++)
		{
			AnimeObj::AnimationStackInfo info;
			info.name = animStack->operator[](i).name;
			info.end = animStack->operator[](i).end;
			info.start = animStack->operator[](i).start;
			((AnimeObj*)*pModelObj)->m_animationStacks.push_back(info);
		}
		//add data in array
		m_modelDataList.push_back(*pModelObj);
	}
	break;
	case ObjDesc::OBJ_TYPE::TERRAIN_OBJECT:
	{
		D3DModelInF* model = NULL;
		if (m_modelObjectList.find(pDesc.terrainDesc.file) == m_modelObjectList.end())
		{
			model = new D3D11TerrainModel();
			D3D11TerrainModelParameterInitial *parameter = new D3D11TerrainModelParameterInitial();
			parameter->arraySize = pDesc.terrainDesc.arraySize;
			parameter->celSize = pDesc.terrainDesc.ceilSize;
			parameter->dimension = pDesc.terrainDesc.dimension;
			parameter->diffuseColor = pDesc.terrainDesc.diffuseColor;
			parameter->diffuseTextureFile = pDesc.terrainDesc.diffuseTextureFile;
			parameter->pDevice = (D3D11Class*)p_device;
			HRESULT hr = model->Initial((char*)pDesc.terrainDesc.file, parameter);
			delete parameter;
			parameter = NULL;
			if (FAILED(hr))
			{
				delete model;
				return false;
			}
			m_modelObjectList[pDesc.terrainDesc.file] = model;
		}
		else
		{
			model = m_modelObjectList[pDesc.terrainDesc.file];
		}
		*pModelObj = new TerrainObj();
		(*pModelObj)->m_modelIndex = string(pDesc.terrainDesc.file);
		m_modelDataList.push_back(*pModelObj);
		
	}
	break;
	case ObjDesc::OBJ_TYPE::OCEAN_OBJECT:
	{
		D3DModelInF* model = NULL;
		if (m_modelObjectList.find(pDesc.oceanDesc.name) == m_modelObjectList.end())
		{
			model = new D3D11OceanModel();
			D3D11OceanModelParameterInitial *parameter = new D3D11OceanModelParameterInitial();
			parameter->arraySize = pDesc.oceanDesc.arraySize;
			parameter->celSize = pDesc.oceanDesc.ceilSize;
			parameter->dimension = pDesc.oceanDesc.dimension;
			parameter->diffuseColor = pDesc.oceanDesc.diffuseColor;
			parameter->pDevice = (D3D11Class*)p_device;
			parameter->normalTexture = pDesc.oceanDesc.normalTexture;
			parameter->roughnessTexture = pDesc.oceanDesc.roughnessTexture;
			parameter->metalTexture = pDesc.oceanDesc.metalTexture;
			HRESULT hr = model->Initial(NULL, parameter);
			delete parameter;
			parameter = NULL;
			if (FAILED(hr))
			{
				delete model;
				return false;
			}
			m_modelObjectList[pDesc.oceanDesc.name] = model;
		}
		else
		{
			model = m_modelObjectList[pDesc.oceanDesc.name];
		}
		*pModelObj = new OceanObj();
		(*pModelObj)->m_modelIndex = string(pDesc.oceanDesc.name);
		m_modelDataList.push_back(*pModelObj);
	}
	break;
	default:
		break;
	}

	return true;
}
bool ObjManager::AddObj(ModelInF* pModelObj)
{

	return false;
}
bool ObjManager::RemoveObj(ModelInF* pModelObj)
{

	return false;
}
HRESULT ObjManager::Initial(DXInF* pDevice)
{
	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	p_fbxManager = FbxManager::Create();
	if (p_fbxManager == NULL) 
	{
		return S_FALSE;
	}
	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(p_fbxManager, IOSROOT);
	if (ios == NULL)
	{
		return S_FALSE;
	}
	p_fbxManager->SetIOSettings(ios);

	//save engine's pointer 
	p_device = pDevice;
	return S_OK;
}
void ObjManager::Destroy()
{
	for (auto &i : m_modelObjectList)
	{
		delete i.second;
		i.second = NULL;
	}

	if (p_fbxManager)
	{
		p_fbxManager->Destroy();
	}
}
std::vector<ModelInF*>* ObjManager::GetModelDataList()
{
	return &m_modelDataList;
}
std::map<std::string, D3DModelInF*>* ObjManager::GetModelObjectList()
{
	return &m_modelObjectList;
}