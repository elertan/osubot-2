// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <thread>
#include "OsuBot.h"

HWND hWnd;
HMODULE hModule;

typedef DWORD PID, *PPID;

BOOL CALLBACK GetMainHwndEnumWindowsProc(const HWND hwnd, const LPARAM lParam)
{
	DWORD dw_process_id(NULL);

	GetWindowThreadProcessId(hwnd, &dw_process_id);

	if (dw_process_id == lParam)
	{
		::hWnd = hwnd;
		return FALSE;
	}

	return TRUE;
}

void GetMainHwnd()
{
	EnumWindows(GetMainHwndEnumWindowsProc, GetCurrentProcessId());
}

DWORD WINAPI MainThread(LPVOID param)
{
	GetMainHwnd();

	OsuBot::run(hModule, hWnd);

	FreeLibraryAndExitThread(hModule, 0);
}


BOOL APIENTRY DllMain(const HMODULE hModule,
                      const DWORD  ulReasonForCall,
                      // ReSharper disable once CppParameterNeverUsed
					  const LPVOID lpReserved)
{
    switch (ulReasonForCall)
    {
    case DLL_PROCESS_ATTACH:
		::hModule = hModule;
		CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(MainThread), nullptr, 0, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    default: ;
    }
    return TRUE;
}

