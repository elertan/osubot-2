#pragma once

#include <Windows.h>
#include <iostream>

class SignatureScanner
{
public:
	SignatureScanner();
	~SignatureScanner();
	bool scan(const uint8_t patternBytes[], const char *patternMask, int patternLength, short patternOffset, DWORD *outAddress) const;
private:
	static bool shouldReadMemory(const void *address, DWORD *outDwRegionSize);
};

