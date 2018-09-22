#include "stdafx.h"
#include "MouseHook.h"

HWND MouseHook::_hWnd;
MouseHook::PCallback MouseHook::_callback;
HHOOK MouseHook::_hookId;

LRESULT WINAPI MouseHook::HookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code > 0)
	{
		if (GetForegroundWindow() == _hWnd)
		{
			PMSLLHOOKSTRUCT mouseInfo = (PMSLLHOOKSTRUCT)lParam;
			auto blockInput = _callback(mouseInfo);
			if (blockInput) return TRUE;
		}
	}
	return CallNextHookEx(_hookId, code, wParam, lParam);
}

void MouseHook::Hook(HWND hWnd, PCallback callback)
{
	_hWnd = hWnd;
	_callback = callback;
	_hookId = SetWindowsHookEx(WH_MOUSE, HookProc, nullptr, 0);
}

void MouseHook::Unhook()
{
	UnhookWindowsHookEx(_hookId);
}
