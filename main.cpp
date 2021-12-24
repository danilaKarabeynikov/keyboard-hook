#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"
#define button1 1
#define button2 2
UINT uiKeyboardMessage = WM_NULL;

HMODULE hHookDLL = NULL;
UINT(CALLBACK *GetInjectionsCount)() = NULL;
BOOL(CALLBACK *SetKeyboardHook)() = NULL;
VOID(CALLBACK *UnhookKeyboardHook)() = NULL;

BOOL bHooked = FALSE;
WPARAM wHookParam = 0;
LPARAM lHookParam = 0;


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
	case WM_CREATE:
		uiKeyboardMessage = RegisterWindowMessage(TEXT("DeniKeyboardHook"));
		CreateWindow(TEXT("button"), TEXT("SetHook"), WS_VISIBLE | WS_CHILD, 0, 0, 160, 40, hWnd, (HMENU)button1, NULL, NULL);
		CreateWindow(TEXT("button"), TEXT("UnHook"), WS_VISIBLE | WS_CHILD, 230, 0, 160, 40, hWnd, (HMENU)button2, NULL, NULL);
		break;

	case WM_PAINT:
	{
		static TCHAR szText[256] = { 0 };
		RECT rc = { 0 };GetClientRect(hWnd, &rc);
		PAINTSTRUCT ps = { 0 };	HDC hdc = BeginPaint(hWnd, &ps);
		if (bHooked){
			UINT nInjectionCount = GetInjectionsCount ? GetInjectionsCount() : 0;
			_stprintf_s(szText, TEXT("Count Injection: %u "), nInjectionCount);
			DrawText(hdc, szText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		if (bHooked && wHookParam){
			rc.top += 48;
			_stprintf_s(szText, TEXT("VKcode: %05X."), wHookParam);
			DrawText(hdc, szText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_COMMAND:
		if (LOWORD(wParam == button1)) {
			if (!hHookDLL)
			{
				hHookDLL = LoadLibrary(TEXT("KeyboardHook.dll"));
				if (hHookDLL)
				{
					(FARPROC&)GetInjectionsCount = GetProcAddress(hHookDLL, "GetInjectionsCount");
					(FARPROC&)SetKeyboardHook = GetProcAddress(hHookDLL, "SetKeyboardHook");
					(FARPROC&)UnhookKeyboardHook = GetProcAddress(hHookDLL, "UnhookKeyboardHook");
				}
				if (SetKeyboardHook)
					bHooked = SetKeyboardHook();
				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		}
		if (LOWORD(wParam == button2)){
			if (hHookDLL)
			{
				if (UnhookKeyboardHook)
					UnhookKeyboardHook();
				bHooked = FALSE;
				UnhookKeyboardHook = NULL;
				SetKeyboardHook = NULL;
				GetInjectionsCount = NULL;
				FreeLibrary(hHookDLL);
				hHookDLL = NULL;
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;


	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	if (uiKeyboardMessage == uiMsg)
	{
		wHookParam = wParam;
		lHookParam = lParam;
		InvalidateRect(hWnd, 0, TRUE);
		return 0;
	}

	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	HANDLE hMutex = CreateMutex(0, FALSE, TEXT("HookLauncherMutex"));
	if (!hMutex)
		return 0;
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	WNDCLASSEX wcx = { 0 };
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = WindowProc;
	wcx.hInstance = hInstance;
	wcx.hIcon = LoadIcon(NULL, IDI_INFORMATION);
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcx.lpszMenuName = MAKEINTRESOURCE(IDM_MAINMENU);
	wcx.lpszClassName = TEXT("SomeClassName");

	if (RegisterClassEx(&wcx) == 0)
		return 1;

	HWND hWnd = CreateWindowEx(0, TEXT("SomeClassName"), TEXT("Keyboard Hook"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 400, 200, NULL, NULL, hInstance, 0);

	if (NULL == hWnd)
		return 2;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (hHookDLL)
	{
		if (UnhookKeyboardHook)
			UnhookKeyboardHook();
		FreeLibrary(hHookDLL);
	}

	return 0;
}