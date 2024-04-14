#pragma once

// used: callvfunc
#include "../../utilities/memory.h"

class CEconItemDefinition;

inline constexpr uint64_t Helper_GetAlternateIconKeyForWeaponPaintWearItem(
	uint16_t nDefIdx, uint32_t nPaintId, uint32_t nWear) {
	return (nDefIdx << 16) + (nPaintId << 2) + nWear;
}

struct AlternateIconData_t {
	const char* sSimpleName;
	const char* sLargeSimpleName;

private:
	char pad0[0x8];  // no idea
	char pad1[0x8];  // no idea

};
class CPaintKit {
public:
	char pad_0x0000[0xE0]; //0x0000

	int64_t PaintKitId() {
		return *reinterpret_cast<int64_t*>((uintptr_t)(this));
	}

	const char* PaintKitName() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x8);
	}

	const char* PaintKitDescriptionString() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x10);
	}

	const char* PaintKitDescriptionTag() {
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x18);
	}

	int32_t PaintKitRarity() {
		return *reinterpret_cast<int32_t*>((uintptr_t)(this) + 0x44);
	}

	bool UsesOldModel() {
		return *reinterpret_cast<bool*>((uintptr_t)(this) + 0xB2);
	}
};
#include "../cstrike/sdk/datatypes/utlmap.h"
class CEconItemSchema {
public:
	auto GetAttributeDefinitionInterface(int iAttribIndex) {
		return MEM::CallVFunc<void*, 27U> (this, iAttribIndex);
	}

	auto& GetSortedItemDefinitionMap() {
		return *reinterpret_cast<CUtlMap<int, CEconItemDefinition*>*>(
			(uintptr_t)(this) + 0x128);
	}

	auto& GetAlternateIconsMap() {
		return *reinterpret_cast<CUtlMap<uint64_t, AlternateIconData_t>*>(
			(uintptr_t)(this) + 0x278);
	}

	auto& GetPaintKits() {
		return *reinterpret_cast<CUtlMap<int, CPaintKit*>*>((uintptr_t)(this) +
			0x2F0);
	}
};

class CEconItemSystem {
public:
	auto GetEconItemSchema() {
		return *reinterpret_cast<CEconItemSchema**>((uintptr_t)(this) + 0x8);
	}
};

enum EClientFrameStage : int
{
	FRAME_UNDEFINED = -1,
	FRAME_START,
	// a network packet is being received
	FRAME_NET_UPDATE_START,
	// data has been received and we are going to start calling postdataupdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// data has been received and called postdataupdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// received all packets, we can now do interpolation, prediction, etc
	FRAME_NET_UPDATE_END,
	// start rendering the scene
	FRAME_RENDER_START = 9,
	// finished rendering the scene
	FRAME_RENDER_END,
	FRAME_NET_FULL_FRAME_UPDATE_ON_REMOVE
};

enum flow : int {
	FLOW_OUTGOING = 0,
	FLOW_INCOMING = 1,
};

class INetChannelInfo {
public:
	float get_latency(flow flow)
	{
		return MEM::CallVFunc<int, 10U>(this, flow);
	}
};

class ISource2Client {
public:
	auto GetEconItemSystem() {
		return MEM::CallVFunc<CEconItemSystem*, 114U>(this);
	}
};

class IEngineClient
{
public:
	int GetMaxClients()
	{
		return MEM::CallVFunc<int, 31U>(this);
	}

	bool IsInGame()
	{
		return MEM::CallVFunc<bool, 32U>(this);
	}

	bool IsConnected()
	{
		return MEM::CallVFunc<bool, 33U>(this);
	}
	//gusta
	INetChannelInfo* GetNetChannelInfo(int split_screen_slot) {
		return MEM::CallVFunc<INetChannelInfo*, 34U>(this, split_screen_slot);
	}
	// return CBaseHandle index
	int GetLocalPlayer()
	{
		int nIndex = -1;

		MEM::CallVFunc<void, 44U>(this, std::ref(nIndex), 0);

		return nIndex + 1;
	}

	[[nodiscard]] const char* GetLevelName()
	{
		return MEM::CallVFunc<const char*, 53U>(this);
	}
	 
	[[nodiscard]] const char* GetLevelNameShort()
	{
		return MEM::CallVFunc<const char*, 54U>(this);
	}

	[[nodiscard]] const char* GetProductVersionString()
	{
		return MEM::CallVFunc<const char*, 77U>(this);
	}

};