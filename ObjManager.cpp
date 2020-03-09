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
bool ObjManager::AddObj(char* file, ModelInF* pModelObj)
{
	D3DModelInF* model = new D3D11Model();
	D3D11ModelParameterInitial* parameter = new D3D11ModelParameterInitial();
	parameter->device = (D3D11Class*)p_device;
	parameter->fbxManager = p_fbxManager;
	model->initial(file, parameter);
	//add obj in the map list
	m_modelObjectList[file] = model;
	delete parameter;
	//do it later
	//pModelObj = new ModelInF();
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
	p_fbxManager = FbxManager::Create();
	if (p_fbxManager == NULL) {
		return S_FALSE;
	}
	p_device = pDevice;
	return S_OK;
}
void ObjManager::Destroy()
{
	if (p_fbxManager)
	{
		p_fbxManager->Destroy();
	}
}