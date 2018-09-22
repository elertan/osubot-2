#include "stdafx.h"
#include "SignatureScanner.h"

#include <psapi.h>

#include <iostream>

using namespace std;


SignatureScanner::SignatureScanner()
{
}


SignatureScanner::~SignatureScanner()
{
}

/*
patternBytes:
Pattern in bytes
patternMask:
Mask in string, char 'x' marking the byte as a must, char '?' marking the byte as wildcard
patternLength:
Specifies the length of the pattern
patternOffset:
Offset of the found address
outAddress:
If the signature matches, this will be filled with the address
*/
bool SignatureScanner::scan(const uint8_t patternBytes[], const char *patternMask, const int patternLength, const short patternOffset, DWORD *outAddress) const
{
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	const auto base_address = reinterpret_cast<DWORD>(sys_info.lpMinimumApplicationAddress);
	const auto scan_size = reinterpret_cast<LONG>(sys_info.lpMaximumApplicationAddress);

	DWORD region_size;

	for (int i = 0; i < scan_size;)
	{
		const bool will_read = this->shouldReadMemory(reinterpret_cast<void*>(base_address + i), &region_size);

		const int start_address = i;
		const int end_address = i + region_size;

		i += region_size;

		if (!will_read) continue;

		for (int x = start_address; x < end_address; x++)
		{
			bool pattern_matches = true;
			for (int pI = 0; pI < patternLength; pI++)
			{
				// Ignore checking for wildcard
				if (patternMask[pI] == '?') continue;

				// Does the byte in memory not match the given pattern byte
				if (*(reinterpret_cast<uint8_t *>(base_address) + x + pI) != patternBytes[pI])
				{
					pattern_matches = false;
					break;
				}
			}
			if (pattern_matches)
			{
				*outAddress = base_address + x + patternOffset;
				return true;
			}
		}
	}

	return false;
}

bool SignatureScanner::shouldReadMemory(const void * address, DWORD *outDwRegionSize)
{
	MEMORY_BASIC_INFORMATION mem_info;
	VirtualQuery(address, &mem_info, sizeof(MEMORY_BASIC_INFORMATION));

	*outDwRegionSize = mem_info.RegionSize;

	return mem_info.State == MEM_COMMIT
		&& !(mem_info.Protect & PAGE_GUARD)
		&& mem_info.Protect > PAGE_NOACCESS
		&& mem_info.Type == MEM_PRIVATE;
}
