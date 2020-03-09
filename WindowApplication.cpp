/*
	Author: Zhuang Theerapong

*/

#include "WindowApplication.h"
#include "MainEditor.h"
#include <timeapi.h>
#include <string>
#include "imGUI\imgui.h"
//global variable
float g_deltaTime;
HWND WindowApplication::m_hwnd = nullptr;
int WindowApplication::Run(ScreenInF* pScreen,HINSTANCE hInstance, int nCmdShow)
{
	m_hwnd = nullptr;


	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "DXSampleClass";
	if (!RegisterClassEx(&windowClass))
		return E_FAIL;
	//get destop size
	RECT windowRect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &windowRect, 0);
	//AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	// Create the window and store a handle to it.
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		"Game Engine",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		hInstance,
		nullptr);
	HRESULT hr;
	hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		return 0;
	}

	//full screen
	nCmdShow = SW_SHOWMAXIMIZED;
	ShowWindow(m_hwnd, nCmdShow);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);
	//get the real window size because the size is changed by fullscreen mode
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	if (!SUCCEEDED(pScreen->OnInit(&m_hwnd, hInstance, width, height))) {
		return 0;
	}

	DWORD dwExecLastTime;
	DWORD dwFPSLastTime;
	DWORD dwCurrentTime;
	DWORD dwFrameCount;
	int	g_nCountFPS = 0;

	timeBeginPeriod(1);	// 分解能を設定
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// システム時刻をミリ秒単位で取得
	dwCurrentTime = dwFrameCount = 0;

	// Main sample loop.
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {

			dwCurrentTime = timeGetTime();					// システム時刻を取得

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)	// 1秒ごとに実行
			{
				g_nCountFPS = dwFrameCount;
				dwFPSLastTime = dwCurrentTime;				// FPSを測定した時刻を保存
				dwFrameCount = 0;
				SetWindowTextA(m_hwnd, ("fps: " + std::to_string(g_nCountFPS) + " DirectX11 Version").c_str());
			}


			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 120))	// 1/60秒ごとに実行
			{
				g_deltaTime = (float)(dwCurrentTime - dwExecLastTime) / 1000;
				//OutputDebugString((std::to_wstring(deltaTime) + L"\n").c_str());
				dwExecLastTime = dwCurrentTime;	// 処理した時刻を保存
				pScreen->OnUpdate();
				pScreen->OnRender(m_hwnd);
				dwFrameCount++;		// 処理回数のカウントを加算
			}

		}
	}
	pScreen->OnDestroy();
	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK WindowApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;


	switch (message)
	{
	case WM_CREATE:
	{

	}
	return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_MOUSEMOVE:
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256)
		{
			if (ImGui::GetCurrentContext() != NULL)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[wParam] = 1;
			}
		}

		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256)
		{
			if (ImGui::GetCurrentContext() != NULL) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[wParam] = 0;
			}
		}
		return 0;
	case WM_CHAR:
		if (ImGui::GetCurrentContext() != NULL) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddInputCharacter((unsigned int)wParam);
		}
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	// Handle any messages the switch statement didn't.
	return 0;
}