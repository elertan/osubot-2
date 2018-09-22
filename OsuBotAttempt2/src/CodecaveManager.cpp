#include "stdafx.h"
#include "CodecaveManager.h"
#include <string>
#include <vector>
#include <iostream>

#define JMP_BYTES_LENGTH 5

CodecaveManager& CodecaveManager::getInstance()
{
	static CodecaveManager instance;
	return instance;
}

CodecaveManager::Codecave CodecaveManager::place(PBYTE address, const uint8_t opcodeLength, const CODECAVE_FUNC detourFunc) const
{
	if (opcodeLength < 5) 
		throw std::exception("opcodeLength needs to be at least 5 bytes in order to place a jmp");

	// Set memory protection
	DWORD old_protect;
	if (!VirtualProtect(address, opcodeLength, PAGE_EXECUTE_READWRITE, &old_protect))
	{
		std::string exMsg = "Couldn't change memory protection at address " + std::to_string(reinterpret_cast<DWORD>(address));
		throw std::exception(exMsg.c_str());
	}

	const auto ptr = static_cast<uint8_t*>(address);
	
	// Store original bytes
	std::vector<uint8_t> org_opcodes(opcodeLength);
	for (uint8_t i = 0; i < opcodeLength; ++i)
	{
		org_opcodes[i] = *(ptr + i);
	}

	// Place CALL
	*address = 0x9A;

	*(reinterpret_cast<PDWORD>(address + 0x1)) = reinterpret_cast<intptr_t>(detourFunc);

	// NOP left over instructions
	for (uint8_t i = JMP_BYTES_LENGTH; i < opcodeLength; ++i)
	{
		*(address + i) = 0x90;
	}

	DWORD _;
	// Restore protection
	if (!VirtualProtect(address, opcodeLength, old_protect, &_))
	{
		std::string exMsg = "Couldn't change memory protection at address " + std::to_string(reinterpret_cast<DWORD>(address));
		throw std::exception(exMsg.c_str());
	}

	Codecave c;
	c.address = address;
	c.orgOpcodes = org_opcodes;
	c.detourFunc = detourFunc;

	return c;
}

void CodecaveManager::restore(const Codecave& c) const
{
	// Set memory protection
	DWORD old_protect;
	if (!VirtualProtect(c.address, c.orgOpcodes.size(), PAGE_EXECUTE_READWRITE, &old_protect))
	{
		std::string exMsg = "Couldn't change memory protection at address " + std::to_string(reinterpret_cast<DWORD>(c.address));
		throw std::exception(exMsg.c_str());
	}

	const auto ptr = static_cast<uint8_t*>(c.address);

	// restore instructions
	for (uint8_t i = 0; i < c.orgOpcodes.size(); ++i)
	{
		*(ptr + i) = c.orgOpcodes[i];
	}

	// Restore protection
	if (!VirtualProtect(c.address, c.orgOpcodes.size(), old_protect, nullptr))
	{
		std::string exMsg = "Couldn't change memory protection at address " + std::to_string(reinterpret_cast<DWORD>(c.address));
		throw std::exception(exMsg.c_str());
	}
}

CodecaveManager::CodecaveManager()
{
}


CodecaveManager::~CodecaveManager()
{
}
