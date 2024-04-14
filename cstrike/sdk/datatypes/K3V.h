#pragma once
#include <tchar.h>
#include <wchar.h>
#include <stdint.h>
#include "../cstrike/common.h"
#include <cstddef>
class CKeyValues3
{
public:
	uint64_t uKey;
	void* pValue;

	std::byte pad[0x8];
};

struct KV3IVD_t
{
	const char* szName;
	uint64_t unk0;
	uint64_t unk1;
};
