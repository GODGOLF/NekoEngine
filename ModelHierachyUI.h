#ifndef _MODEL_HIERACHY_H_
#define _MODEL_HIERACHY_H_
#include "EditorInF.h"
#include "ModelProperty.h"
#include "ObjSceneInF.h"
#include <vector>

class ModelHierachyUI : public EditorInf
{
public:
	ModelHierachyUI();
	~ModelHierachyUI();
	bool Initial(std::vector<ModelProperty>* models, ObjScene* manager);
	void Render(Camera* camera) override;
	void Destroy();
private:
	void HierachyWindow(Camera* camera);
	void AddWindow();
	void WarnmingMessageWindow();
private:
	bool m_showAddWindow;
	bool m_showMessagebox;
	std::vector<ModelProperty>* m_models;
	ObjScene* m_objManager;
	void LoadModelListFromFolder(std::vector<char*> &output);
	
};
#endif // !_MODEL_HIERACHY_H_
