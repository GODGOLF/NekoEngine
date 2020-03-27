#include "MenuBarEditor.h"
#include "imGUI\imgui.h"

void MenuBarEditor::Render(Camera* camera)
{
	static bool save;
	static bool load;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("save", "", save))
			{

			}
			if (ImGui::MenuItem("load", "", load))
			{

			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}