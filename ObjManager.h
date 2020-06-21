#pragma once

#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_

#include "ObjSceneInF.h"
#include "DXInF.h"
#include "D3DModelInF.h"
#include <vector>
#include <map>
#include <fbxsdk.h>

class ObjManager : public ObjScene
{
public:
	ObjManager();
	virtual ~ObjManager();
	bool AddObj(ModelInF** pModelObj, ObjDesc desc) override;
	bool RemoveObj(ModelInF* pModelObj) override;
	HRESULT Initial(DXInF* pDevice);
	void Destroy() override;
	std::vector<ModelInF*>* GetModelDataList();
	std::map<std::string, D3DModelInF*>* GetModelObjectList();
private:
	DXInF * p_device;
	std::vector<ModelInF*> m_modelDataList;
	std::map<std::string, D3DModelInF*> m_modelObjectList;
	FbxManager* p_fbxManager;
};


#endif // !_MODEL_MANAGER_H_

