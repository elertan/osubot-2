// dllmain.cpp : Defines the entry point for the DLL application.
#include "Bot.h"

HMODULE hModule;

DWORD WINAPI MainThread(LPVOID param)
{
	osubot::Run();
	FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(const HMODULE hModule,
	const DWORD  ul_reason_for_call,
	// ReSharper disable once CppParameterNeverUsed
	LPVOID lpReserved
)
{
	// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		::hModule = hModule;
		CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(MainThread), nullptr, 0, nullptr);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}