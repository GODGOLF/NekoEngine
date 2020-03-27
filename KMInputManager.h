/*
KeyInputManager.h

Author: Zhuang Theerapong

*/
#ifndef _KM_INPUT_MANAGER_H_
#define _KM_INPUT_MANAGER_H_
#pragma once
#define DIRECTINPUT_VERSION 0x0800
#define	NUM_KEY_MAX			(256)
#include <dinput.h>
class KMInputManager {
public:
	KMInputManager();
	virtual ~KMInputManager();
	bool Initialize(HINSTANCE, HWND*, int, int);
	void Shudown();
	bool Frame();
	bool GetKeyboardPress(int key);
	bool GetKeyboardTrigger(int key);
	bool GetKeyboardRepeat(int key);
	bool GetKeyboardRelease(int key);
	void GetMouseLocation(int& mouseX, int& mouseY);
	void GetMouseFrameRate(int& mouseX, int& mouseY);
	bool GetMouseLeftClick();
	bool GetMouseRightClick();
	bool GetMouseMiddleClick();
	int GetMouseWheel();

	DIMOUSESTATE m_mouseState;


private:
	IDirectInput8 * m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;
	bool ReadKeyboard();
	bool ReadMouse();
	BYTE m_keyState[NUM_KEY_MAX];
	BYTE m_keyStateTrigger[NUM_KEY_MAX];		// キーボードの状態を受け取るワーク
	BYTE m_keyStateRepeat[NUM_KEY_MAX];		// キーボードの状態を受け取るワーク
	BYTE m_keyStateRelease[NUM_KEY_MAX];		// キーボードの状態を受け取るワーク
	int	 m_keyStateRepeatCnt[NUM_KEY_MAX];	// キーボードのリピートカウンタ
	HWND* hwnd;
};
#endif // !KEYINPUTMANAGER




