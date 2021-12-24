#include <windows.h>

#pragma data_seg(".DENISEG")
UINT nInjectionsCount = 0;
#pragma data_seg()

HINSTANCE hInstance = NULL;
HWND hLauncherWnd = NULL;
UINT uiKeyboardMessage = WM_NULL;
HHOOK hKeyboardHook = NULL;

UINT CALLBACK GetInjectionsCount()
{
	return nInjectionsCount;
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION && hLauncherWnd)
		PostMessage(hLauncherWnd, uiKeyboardMessage, wParam, lParam);
	
	return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
}

BOOL CALLBACK SetKeyboardHook()
{
	if (!hKeyboardHook)
		hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardProc, hInstance, 0);
	return hKeyboardHook ? TRUE : FALSE;
}

VOID CALLBACK UnhookKeyboardHook()
{
	if (hKeyboardHook)
		UnhookWindowsHookEx(hKeyboardHook);
	hKeyboardHook = NULL;
}

BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		hInstance = hDLL;
		InterlockedIncrement(&nInjectionsCount);
		hLauncherWnd = FindWindow(TEXT("SomeClassName"), NULL);
		uiKeyboardMessage = RegisterWindowMessage(TEXT("DeniKeyboardHook"));
		break;
		
	case DLL_PROCESS_DETACH:
		InterlockedDecrement(&nInjectionsCount);
		break;
	}
	return TRUE;
}