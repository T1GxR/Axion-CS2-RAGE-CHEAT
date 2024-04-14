#pragma once
#include "utlmemory.h"

// used: memorymove
#include "../../utilities/fnv1a.h"

#define STRINGTOKEN_MURMURHASH_SEED 0x31415926

#pragma pack(push, 8)
class CUtlStringToken
{
public:
	explicit CUtlStringToken(const char* szKeyName)
	{
		uHashCode = FNV1A::Hash(szKeyName, STRINGTOKEN_MURMURHASH_SEED);
		szDebugName = szKeyName;
	}

	constexpr CUtlStringToken(const FNV1A_t uHashCode, const char* szKeyName) :
		uHashCode(uHashCode), szDebugName(szKeyName) { }

	CS_INLINE bool operator==(const CUtlStringToken& other) const
	{
		return (other.uHashCode == uHashCode);
	}

	CS_INLINE bool operator!=(const CUtlStringToken& other) const
	{
		return (other.uHashCode != uHashCode);
	}

	CS_INLINE bool operator<(const CUtlStringToken& other) const
	{
		return (uHashCode < other.uHashCode);
	}

public:
	FNV1A_t uHashCode = 0U; // 0x00
	const char* szDebugName = nullptr; // 0x08 //   @Todo: for some reason retards keep this even for non-debug builds, it can be changed later
};
#pragma pack(pop)

// helper to create a string token at compile-time
CS_INLINE consteval CUtlStringToken MakeStringToken(const char* szKeyName)
{
	return { FNV1A::HashConst(szKeyName, STRINGTOKEN_MURMURHASH_SEED), szKeyName };
}
