#pragma once
class MouseHook
{
public:
	MouseHook() = delete;
	typedef bool(*PCallback)(const PMSLLHOOKSTRUCT hookStruct);
	static void Hook(HWND hWnd, PCallback callback);
	static void Unhook();
private:
	static LRESULT WINAPI HookProc(int code, WPARAM wParam, LPARAM lParam);
	static HWND _hWnd;
	static PCallback _callback;
	static HHOOK _hookId;
};

