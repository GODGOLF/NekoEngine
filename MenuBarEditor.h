#pragma once
#ifndef _MENU_BAR_EDITOR_H_
#define _MENU_BAR_EDITOR_H_
#include "EditorInf.h"

class MenuBarEditor : public EditorInf
{
public:
	void OnInitial();
	void Render(Camera* camera) override;

};

#endif // !_MENU_BAR_EDITOR_H_

