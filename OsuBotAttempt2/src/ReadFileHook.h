#pragma once
#include <string>

typedef void(*PREAD_FILE_HOOK_READ_FILE_CALLBACK)(std::wstring filepath);

class ReadFileHook
{
public:
	static void hook(PREAD_FILE_HOOK_READ_FILE_CALLBACK readFileCallback);
	static void unhook();
	static bool is_hooked;
private:
	ReadFileHook() = delete;
	typedef DWORD(WINAPI *P_READ_FILE)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesToRead, LPOVERLAPPED lpOverlapped);
	static DWORD WINAPI readFileHk(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesToRead, LPOVERLAPPED lpOverlapped);
	static P_READ_FILE org_read_file_;
	static PREAD_FILE_HOOK_READ_FILE_CALLBACK callback_;
};

