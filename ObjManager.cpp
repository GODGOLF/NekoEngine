#include "DXInclude.h"
#include "ObjManager.h"
#include "D3D11Model.h"

ObjManager::ObjManager() :p_device(NULL),p_fbxManager(NULL)
{
	
}
ObjManager::~ObjManager()
{
	Destroy();
}
bool ObjManager::AddObj(char* file, ModelInF** pModelObj)
{
	D3DModelInF* model = new D3D11Model();
	D3D11ModelParameterInitial* parameter = new D3D11ModelParameterInitial();
	parameter->pDevice = (D3D11Class*)p_device;
	parameter->pFbxManager = p_fbxManager;
	model->Initial(file, parameter);
	//add obj in the map list
	m_modelObjectList[file] = model;
	delete parameter;
	//create obj data
	*pModelObj = new ModelInF();
	(*pModelObj)->modelIndex = string(file);
	//add data in array
	m_modelDataList.push_back(*pModelObj);
	return false;
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