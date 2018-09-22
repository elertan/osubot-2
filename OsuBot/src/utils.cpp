#include "stdafx.h"
#include "Utils.hpp"

#define NOMINMAX
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <tchar.h>
#include <psapi.h>
#include <strsafe.h>

#define BUFSIZE 512

HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;

namespace Utils
{
	/*
	* @brief Create console
	*
	* Create and attach a console window to the current process
	*/
	void AttachConsole()
	{
		_old_out = GetStdHandle(STD_OUTPUT_HANDLE);
		_old_err = GetStdHandle(STD_ERROR_HANDLE);
		_old_in = GetStdHandle(STD_INPUT_HANDLE);

		::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

		_out = GetStdHandle(STD_OUTPUT_HANDLE);
		_err = GetStdHandle(STD_ERROR_HANDLE);
		_in = GetStdHandle(STD_INPUT_HANDLE);

		// Bind to the console
		freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
		freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

		SetConsoleMode(_out,
			ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

		SetConsoleMode(_in,
			ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
			ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
	}

	/*
	* @brief Detach console
	*
	* Detach and destroy the attached console
	*/
	void DetachConsole()
	{
		if (_out && _err && _in) {
			FreeConsole();

			if (_old_out)
				SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
			if (_old_err)
				SetStdHandle(STD_ERROR_HANDLE, _old_err);
			if (_old_in)
				SetStdHandle(STD_INPUT_HANDLE, _old_in);
		}
	}

	/*
	* @brief Print to console
	*
	* Replacement to printf that works with the newly created console
	*/
	bool ConsolePrint(const char* fmt, ...)
	{
		if (!_out)
			return false;

		char buf[1024];
		va_list va;

		va_start(va, fmt);
		_vsnprintf_s(buf, 1024, fmt, va);
		va_end(va);

		return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
	}

	/*
	* @brief Print to console with a newline
	*
	* Replacement to printf that works with the newly created console
	*/
	bool ConsolePrintLn(const char* fmt, ...)
	{
		std::string str(fmt);
		str.append("\n");
		const char* cstr = str.c_str();

		if (!_out)
			return false;

		char buf[1024];
		va_list va;

		va_start(va, cstr);
		_vsnprintf_s(buf, 1024, cstr, va);
		va_end(va);

		return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
	}

	/*
	* @brief Blocks execution until a key is pressed on the console window
	*
	*/
	char ConsoleReadKey()
	{
		if (!_in)
			return false;

		auto key = char{ 0 };
		auto keysread = DWORD{ 0 };

		ReadConsoleA(_in, &key, 1, &keysread, nullptr);

		return key;
	}


	/*
	* @brief Wait for all the given modules to be loaded
	*
	* @param timeout How long to wait
	* @param modules List of modules to wait for
	*
	* @returns See WaitForSingleObject return values.
	*/
	int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules)
	{
		bool signaled[32] = { 0 };
		bool success = false;

		std::uint32_t totalSlept = 0;

		if (timeout == 0) {
			for (auto& mod : modules) {
				if (GetModuleHandleW(std::data(mod)) == NULL)
					return WAIT_TIMEOUT;
			}
			return WAIT_OBJECT_0;
		}

		if (timeout < 0)
			timeout = INT32_MAX;

		while (true) {
			for (auto i = 0u; i < modules.size(); ++i) {
				auto& module = *(modules.begin() + i);
				if (!signaled[i] && GetModuleHandleW(std::data(module)) != NULL) {
					signaled[i] = true;

					//
					// Checks if all modules are signaled
					//
					bool done = true;
					for (auto j = 0u; j < modules.size(); ++j) {
						if (!signaled[j]) {
							done = false;
							break;
						}
					}
					if (done) {
						success = true;
						goto exit;
					}
				}
			}
			if (totalSlept > std::uint32_t(timeout)) {
				break;
			}
			Sleep(10);
			totalSlept += 10;
		}

	exit:
		return success ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
	}

	/*
	* @brief Scan for a given byte pattern on a module
	*
	* @param module    Base of the module to search
	* @param signature IDA-style byte array pattern
	*
	* @returns Address of the first occurence
	*/
	std::uint8_t* PatternScan(void* module, const char* signature)
	{
		static auto pattern_to_byte = [](const char* pattern) {
			auto bytes = std::vector<int>{};
			auto start = const_cast<char*>(pattern);
			auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current) {
				if (*current == '?') {
					++current;
					if (*current == '?')
						++current;
					bytes.push_back(-1);
				}
				else {
					bytes.push_back(strtoul(current, &current, 16));
				}
			}
			return bytes;
		};

		auto dosHeader = (PIMAGE_DOS_HEADER)module;
		auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

		auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto patternBytes = pattern_to_byte(signature);
		auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

		auto s = patternBytes.size();
		auto d = patternBytes.data();

		for (auto i = 0ul; i < sizeOfImage - s; ++i) {
			bool found = true;
			for (auto j = 0ul; j < s; ++j) {
				if (scanBytes[i + j] != d[j] && d[j] != -1) {
					found = false;
					break;
				}
			}
			if (found) {
				return &scanBytes[i];
			}
		}
		return nullptr;
	}
	std::wstring GetDOSFilePathByHandle(HANDLE hFile)
	{
		BOOL bSuccess = FALSE;
		TCHAR pszFilename[MAX_PATH + 1];
		HANDLE hFileMap;

		// Get the file size.
		DWORD dwFileSizeHi = 0;
		DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

		if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
		{
			_tprintf(TEXT("Cannot map a file with a length of zero.\n"));
			return FALSE;
		}

		// Create a file mapping object.
		hFileMap = CreateFileMapping(hFile,
			NULL,
			PAGE_READONLY,
			0,
			1,
			NULL);

		if (hFileMap)
		{
			// Create a file mapping to get the file name.
			void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

			if (pMem)
			{
				if (GetMappedFileName(GetCurrentProcess(),
					pMem,
					pszFilename,
					MAX_PATH))
				{

					// Translate path with device name to drive letters.
					TCHAR szTemp[BUFSIZE];
					szTemp[0] = '\0';

					if (GetLogicalDriveStrings(BUFSIZE - 1, szTemp))
					{
						TCHAR szName[MAX_PATH];
						TCHAR szDrive[3] = TEXT(" :");
						BOOL bFound = FALSE;
						TCHAR* p = szTemp;

						do
						{
							// Copy the drive letter to the template string
							*szDrive = *p;

							// Look up each device name
							if (QueryDosDevice(szDrive, szName, MAX_PATH))
							{
								size_t uNameLen = _tcslen(szName);

								if (uNameLen < MAX_PATH)
								{
									bFound = _tcsnicmp(pszFilename, szName, uNameLen) == 0
										&& *(pszFilename + uNameLen) == _T('\\');

									if (bFound)
									{
										// Reconstruct pszFilename using szTempFile
										// Replace device path with DOS path
										TCHAR szTempFile[MAX_PATH];
										StringCchPrintf(szTempFile,
											MAX_PATH,
											TEXT("%s%s"),
											szDrive,
											pszFilename + uNameLen);
										StringCchCopyN(pszFilename, MAX_PATH + 1, szTempFile, _tcslen(szTempFile));
									}
								}
							}

							// Go to the next NULL character.
							while (*p++);
						} while (!bFound && *p); // end of string
					}
				}
				bSuccess = TRUE;
				UnmapViewOfFile(pMem);
			}

			CloseHandle(hFileMap);
		}
		return pszFilename;
	}
}