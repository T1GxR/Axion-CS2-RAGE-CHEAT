#pragma once
#include "../../utilities/memory.h"

class CNetworkGameClient
{
public:
	bool IsConnected()
	{
		return MEM::CallVFunc<bool, 13U>(this);
	}

	// force game to clear cache and reset delta tick
	void FullUpdate()
	{
		// @ida: #STR: "Requesting full game update (%s)...\n"
		MEM::CallVFunc<void, 31U>(this, CS_XOR("initial update"));
	}
	void Update()
	{
		int* deltaTickPtr = reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x258);

		*deltaTickPtr = -1;
	}
	int GetDeltaTick()
	{
		// @ida: offset in FullUpdate();
		// (nDeltaTick = -1) == FullUpdate() called
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x258);
	}
};

class INetworkClientService
{
public:
	int deltatick() {
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x258);
	}
	CNetworkGameClient* GetNetworkClient() {
		return MEM::CallVFunc<CNetworkGameClient*, 23>(this);
	}
};