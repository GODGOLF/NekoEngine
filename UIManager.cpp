#include "UIManager.h"
#include "MenuBarEditor.h"

UIManager::UIManager()
{

}
UIManager::~UIManager()
{

}

bool UIManager::Initial()
{
	m_uiInterface.push_back(new MenuBarEditor());
	return true;
}

EditorInf* UIManager::GetUIInterface(int id)
{
	if (id >= m_uiInterface.size())
	{
		return NULL;
	}
	return m_uiInterface[id];
}