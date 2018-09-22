#include "stdafx.h"
#include "ReadFileHook.h"
#include "mhook-lib/mhook.h"
#include "helpers/Utils.hpp"

#define FINAL_PATH_BUFFER_SIZE 512

bool ReadFileHook::is_hooked = false;
ReadFileHook::P_READ_FILE ReadFileHook::org_read_file_;
PREAD_FILE_HOOK_READ_FILE_CALLBACK ReadFileHook::callback_;

void ReadFileHook::hook(const PREAD_FILE_HOOK_READ_FILE_CALLBACK readFileCallback)
{
	callback_ = readFileCallback;

	org_read_file_ = reinterpret_cast<P_READ_FILE>(GetProcAddress(GetModuleHandle(L"kernel32.dll"), "ReadFile"));
	Mhook_SetHook(reinterpret_cast<PVOID*>(&org_read_file_), readFileHk);

	is_hooked = true;
}

void ReadFileHook::unhook()
{
	Mhook_Unhook(reinterpret_cast<PVOID*>(&org_read_file_));

	is_hooked = false;
}

DWORD ReadFileHook::readFileHk(const HANDLE hFile, const LPVOID lpBuffer, const DWORD nNumberOfBytesToRead, const LPDWORD lpNumberOfBytesToRead, const LPOVERLAPPED lpOverlapped)
{
	/*TCHAR path[FINAL_PATH_BUFFER_SIZE];
	GetFinalPathNameByHandle(hFile, path, FINAL_PATH_BUFFER_SIZE, VOLUME_NAME_NT);
	std::wstring nigger(path);*/
	const auto filepath = Utils::GetDOSFilePathByHandle(hFile);
	callback_(filepath);
	return org_read_file_(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesToRead, lpOverlapped);
}
