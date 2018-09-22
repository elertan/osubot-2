#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <map>
#include <Objbase.h>


class CodecaveManager
{
public:
	typedef int CODECAVE_ID;
	typedef void(*CODECAVE_FUNC)();
	struct Codecave
	{
		CODECAVE_ID id;
		PVOID address;
		CODECAVE_FUNC detourFunc;
		std::vector<uint8_t> orgOpcodes;
	};

	static CodecaveManager& getInstance();
	Codecave place(PBYTE address, const uint8_t opcodeLength, const CODECAVE_FUNC detourFunc) const;
	void restore(const Codecave& codecave) const;
private:
	CodecaveManager();
	~CodecaveManager();
};

