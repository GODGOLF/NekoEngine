#ifndef _UI_MANAGER_H_
#define _UI_MANAGER_H_
#include "EditorInF.h" 
#include <vector>

class UIManager
{
public:
	UIManager();
	~UIManager();
	bool Initial();

	EditorInf* GetUIInterface(int id);
private:
	std::vector<EditorInf*> m_uiInterface;

};
#endif // !_UI_MANAGER_H_

