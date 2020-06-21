#include "ModelHierachyUI.h"
#include "imGUI\imgui.h"
#include <string>
#include "FunctionHelper.h"
#define DIRECTORY_SEARCH	"Data/Models/*.fbx"
#define DIRECTORY	"Data/Models/"

ModelHierachyUI::ModelHierachyUI()
{
	m_showAddWindow = false;
}
ModelHierachyUI::~ModelHierachyUI()
{
	Destroy();
}
bool ModelHierachyUI::Initial(std::vector<ModelProperty>* models, ObjScene* manager)
{
	m_models = models;
	m_objManager = manager;
	return true;
}
void ModelHierachyUI::Render(Camera* camera)
{
	HierachyWindow(camera);
	AddWindow();
	WarnmingMessageWindow();
	
}
void ModelHierachyUI::Destroy()
{

}
void ModelHierachyUI::HierachyWindow(Camera* camera)
{
	std::vector<const char*> cName;
	for (unsigned int i = 0; i < m_models->size(); i++)
	{
		cName.push_back(m_models->operator[](i).model->name.c_str());
	}
	static int curSelected = 0;
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin("Tool");
	ImGui::BeginChild("Option", ImVec2(150, 0), true);
	ImGui::Text("Heirachy");
	if (ImGui::ListBox("Model List", &curSelected, cName.data(), cName.size(), 5))
	{

	}
	if (ImGui::Button("add object"))
	{
		m_showAddWindow = true;

	}
	if (ImGui::Button("add collider"))
	{

	}
	if (ImGui::Button("add light")) {


	}
	if (ImGui::Button("delete Object"))
	{
		m_objManager->RemoveObj(m_models->operator[](curSelected).model);
		m_models->erase(m_models->begin() + curSelected);
	}
	static float curPostion[3];
	DirectX::XMVECTOR pos = camera->GetPosition();
	DirectX::XMFLOAT3 fpos;
	DirectX::XMStoreFloat3(&fpos, pos);
	curPostion[0] = fpos.x;
	curPostion[1] = fpos.y;
	curPostion[2] = fpos.z;
	ImGui::Text("Current Position");
	std::string curPos = "X: " + std::to_string(curPostion[0]) + "\nY: " + std::to_string(curPostion[1]) + "\nZ: " + std::to_string(curPostion[2]);
	ImGui::Text(curPos.c_str());
	ImGui::EndChild();

	static float posInfo[3] = { 0,0,0 };
	static float rotInfo[3] = { 0,0,0 };
	static float scaleInfo[3] = { 1,1,1 };
	posInfo[0] = m_models->operator[](curSelected).model->position.x;
	posInfo[1] = m_models->operator[](curSelected).model->position.y;
	posInfo[2] = m_models->operator[](curSelected).model->position.z;
	DirectX::XMFLOAT3 rotation = m_models->operator[](curSelected).model->GetRotation();
	rotInfo[0] = rotation.x;
	rotInfo[1] = rotation.y;
	rotInfo[2] = rotation.z;
	scaleInfo[0] = m_models->operator[](curSelected).model->scale.x;
	scaleInfo[1] = m_models->operator[](curSelected).model->scale.y;
	scaleInfo[2] = m_models->operator[](curSelected).model->scale.z;
	ImGui::SameLine();
	ImGui::BeginChild("Info", ImVec2(200, 0), true);
	ImGui::Text("Model Infomation");
	ImGui::InputFloat3("Position", &posInfo[0]);
	ImGui::InputFloat3("Rotation", &rotInfo[0]);
	ImGui::InputFloat3("Scale", &scaleInfo[0]);
	m_models->operator[](curSelected).model->position = DirectX::XMFLOAT3(&posInfo[0]);
	m_models->operator[](curSelected).model->scale = DirectX::XMFLOAT3(&scaleInfo[0]);
	m_models->operator[](curSelected).model->SetRotation(rotInfo[0], rotInfo[1], rotInfo[2]);
	ImGui::EndChild();
	ImGui::End();
	
}
void ModelHierachyUI::AddWindow()
{
	if (!m_showAddWindow)
	{
		return;
	}
	static float pos[3] = {0,0,0};
	static float rot[3] = { 0,0,0 };
	static float scale[3] = { 1,1,1 };
	
	ImGui::Begin("AddWindow", &m_showAddWindow);
	ImGui::SetWindowSize(ImVec2(230, 300),ImGuiCond_::ImGuiCond_FirstUseEver);
	static int curSelected;
	std::vector<char*> fileList;
	LoadModelListFromFolder(fileList);
	ImGui::ListBox("Model List", &curSelected, &fileList.data()[0], fileList.size());
	ImGui::InputFloat3("Position",&pos[0]);
	ImGui::InputFloat3("Rotation", &rot[0]);
	ImGui::InputFloat3("Scale", &scale[0]);
	if (ImGui::Button("Create"))
	{
		ModelProperty model;
		ObjDesc obj1Dsc;
		obj1Dsc.type = ObjDesc::MODEL_OBJECT;
		std::string directory = DIRECTORY;
		directory.append(&fileList[curSelected][0]);
		obj1Dsc.modelDesc.file = directory.c_str();
		////test
		bool result = m_objManager->AddObj(&model.model, obj1Dsc);
		if (result)
		{
			m_models->push_back(model);
			model.model->position = DirectX::XMFLOAT3(pos);
			model.model->SetRotation(rot[0], rot[1], rot[2]);
			model.model->scale = DirectX::XMFLOAT3(scale);
		}
		else
		{
			m_showMessagebox = true;
		}
		m_showAddWindow = false;
	}
	if (ImGui::Button("Cancel"))
	{
		m_showAddWindow = false;
	}
	ImGui::End();
	//remove model List data
	for (auto &i : fileList)
	{
		delete[] i;
	}
}
void ModelHierachyUI::LoadModelListFromFolder(std::vector<char*> &output)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(DIRECTORY_SEARCH, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				char* name = new char[255];
				strcpy_s(name,255, fd.cFileName);
				output.push_back(name);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
}
void ModelHierachyUI::WarnmingMessageWindow()
{
	if (!m_showMessagebox)
	{
		return;
	}
	ImGui::Begin("Message", &m_showMessagebox);
	ImGui::SetWindowSize(ImVec2(230, 300), ImGuiCond_::ImGuiCond_FirstUseEver);
	ImGui::Text("Can't read file");
	if (ImGui::Button("OK"))
	{
		m_showMessagebox = false;
	}
	ImGui::End();
}