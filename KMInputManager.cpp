
#include "KMInputManager.h"

KMInputManager::KMInputManager() : m_directInput(NULL),m_keyboard(NULL),m_mouse(NULL)
{

}
bool KMInputManager::Initialize(HINSTANCE hinstance, HWND* hwnd, int screenWidth, int screenHeight) {
	HRESULT result;
	// Initialize the main direct input interface.
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the keyboard.
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the keyboard to not share with other programs.
	result = m_keyboard->SetCooperativeLevel(*hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		OutputDebugString("cooperative level keyboard error\n");
		return false;
	}



	// Initialize the direct input interface for the mouse.
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Set the data format for the mouse using the pre-defined mouse data format.
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the mouse to share with other programs.
	//result = m_mouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	result = m_mouse->SetCooperativeLevel(*hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	//DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
	if (FAILED(result))
	{
		return false;
	}

	this->hwnd = hwnd;
	return true;
}
KMInputManager::~KMInputManager() {

}
//=============================================================================
// キーボードのプレス状態を取得
//=============================================================================
bool KMInputManager::GetKeyboardPress(int key)
{
	return (m_keyState[key] & 0x80) ? true : false;
}

//=============================================================================
// キーボードのトリガー状態を取得
//=============================================================================
bool KMInputManager::GetKeyboardTrigger(int key)
{
	return (m_keyStateTrigger[key] & 0x80) ? true : false;
}

//=============================================================================
// キーボードのリピート状態を取得
//=============================================================================
bool KMInputManager::GetKeyboardRepeat(int key)
{

	return (m_keyStateRepeat[key] & 0x80) ? true : false;
}

//=============================================================================
// キーボードのリリ－ス状態を取得
//=============================================================================
bool KMInputManager::GetKeyboardRelease(int key)
{

	return (m_keyStateRelease[key] & 0x80) ? true : false;
}
void KMInputManager::Shudown() {
	// Release the mouse.
	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}

	// Release the keyboard.
	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}

	// Release the main interface to direct input.
	if (m_directInput)
	{
		m_directInput->Release();
		m_directInput = 0;
	}
	hwnd = 0;
	return;
}
bool KMInputManager::Frame() {
	bool result;


	// Read the current state of the keyboard.
	result = ReadKeyboard();

	// Read the current state of the mouse.
	result = ReadMouse();

	return true;
}
bool KMInputManager::ReadKeyboard()
{
	HRESULT hr;
	BYTE keyStateOld[256];

	// 前回のデータを保存
	memcpy(keyStateOld, m_keyState, NUM_KEY_MAX);
	// デバイスからデータを取得
	hr = m_keyboard->GetDeviceState(sizeof(m_keyState), m_keyState);
	if (SUCCEEDED(hr))
	{
		for (int cnt = 0; cnt < NUM_KEY_MAX; cnt++)
		{
			m_keyStateTrigger[cnt] = (keyStateOld[cnt] ^ m_keyState[cnt]) & m_keyState[cnt];
			m_keyStateRelease[cnt] = (keyStateOld[cnt] ^ m_keyState[cnt]) & ~m_keyState[cnt];
			m_keyStateRepeat[cnt] = m_keyStateTrigger[cnt];

			if (m_keyState[cnt])
			{
				m_keyStateRepeatCnt[cnt]++;
				if (m_keyStateRepeatCnt[cnt] >= 20)
				{
					m_keyStateRepeat[cnt] = m_keyState[cnt];
				}
			}
			else
			{
				m_keyStateRepeatCnt[cnt] = 0;
				m_keyStateRepeat[cnt] = 0;
			}
		}
	}
	else
	{
		// キーボードへのアクセス権を取得
		hr = m_keyboard->Acquire();
	}
	return hr;
}
bool KMInputManager::ReadMouse()
{
	HRESULT result;

	// Read the mouse device.
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}
void KMInputManager::GetMouseFrameRate(int& mouseX, int& mouseY) {
	mouseX = m_mouseState.lX;
	mouseY = m_mouseState.lY;
}
void KMInputManager::GetMouseLocation(int& mouseX, int& mouseY) {
	mouseX = -INT_MAX;
	mouseY = -INT_MAX;
	POINT pos;
	if (HWND active_window = ::GetForegroundWindow())
	{
		if (active_window == *hwnd || ::IsChild(active_window, *hwnd))
		{
			if (::GetCursorPos(&pos) && ::ScreenToClient(*hwnd, &pos))
			{
				mouseX = (int)pos.x;
				mouseY = (int)pos.y;
			}
		}
	}
}
bool KMInputManager::GetMouseLeftClick() {
	return m_mouseState.rgbButtons[0] & 0x80 ? true : false;

}
bool KMInputManager::GetMouseRightClick() {
	return m_mouseState.rgbButtons[1] & 0x80 ? true : false;
}
bool  KMInputManager::GetMouseMiddleClick() {
	return m_mouseState.rgbButtons[2] & 0x80 ? true : false;
}
int KMInputManager::GetMouseWheel() {
	return m_mouseState.lZ;
}


