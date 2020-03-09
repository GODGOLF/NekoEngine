#include "WindowApplication.h"
#include "MainEditor.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	MainEditor editor;
	WindowApplication::Run(&editor, hInstance, nCmdShow);
}