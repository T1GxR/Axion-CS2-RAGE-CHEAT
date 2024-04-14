#pragma once
// used: cutllinkedlist
#include "../datatypes/utllinkedlist.h"
// used: fnv1a hashing
#include "../../utilities/fnv1a.h"

// used: sdk datatypes
#include "../datatypes/color.h"
#include "../datatypes/vector.h"
#include "../datatypes/qangle.h"

// used: call virtual function
#include "../../utilities/memory.h"

// used: color_t
#include "../datatypes/color.h"

// used: cbasehandle
#include "../entity.h"
#include "../datatypes/utlstringtoken.h"
using namespace MEM;

class IGameEvent
{
public:
	virtual ~IGameEvent() { }

	[[nodiscard]] const char* GetName() const
	{
		//   @ida: client.dll -> U8["48 8B CE FF 50 ? 48 8D 0D" + 0x5]   @XREF: "show_freeze_panel"
		return CallVFunc<const char*, 1U>(this);
	}
	
	[[nodiscard]] bool GetBool(const CUtlStringToken& keyToken, bool bDefault = false) const
	{
		return CallVFunc<bool, 6U>(this, &keyToken, bDefault);
	}

	[[nodiscard]] int GetInt(const CUtlStringToken& keyToken, int iDefault = 0) const
	{
		//   @ida CGameEvent::GetInt(const char*, int): client.dll -> ABS["E8 ? ? ? ? 0F B6 4D 77" + 0x1] 
		//   @ida: client.dll -> U8["E8 ? ? ? ? 45 33 C0 48 89 74 24 ? 48 8D 54 24 ? 49 8B CF FF D3 85 C0 8B 44 24 48" - 0x1] / sizeof(std::uintptr_t)   @XREF: "headshot"
		return CallVFunc<int, 7U>(this, &keyToken, iDefault);
	}

	[[nodiscard]] std::uint64_t GetUint64(const CUtlStringToken& keyToken, std::uint64_t ullDefault = 0ULL) const
	{
		return CallVFunc<std::uint64_t, 8U>(this, &keyToken, ullDefault);
	}

	[[nodiscard]] float GetFloat(const CUtlStringToken& keyToken, const float flDefault = 0.0f) const
	{
		//   @ida CGameEvent::GetFloat(const char*, float): client.dll -> ABS["E8 ? ? ? ? 0F 28 D8 89 5C 24 20" + 0x1]
		//   @ida: client.dll -> U8["4C 8B 60 ? 4D 8B C6" + 0x3] / sizeof(std::uintptr_t)   @XREF: "theta"
		return CallVFunc<float, 9U>(this, &keyToken, flDefault);
	}

	[[nodiscard]] const char* GetString(const CUtlStringToken& keyToken, const char* szDefault = "") const
	{
		//   @ida: client.dll -> U8["48 8B 78 ? 48 8D 4D 80" + 0x3] / sizeof(std::uintptr_t)   @XREF: "weapon"
		return CallVFunc<const char*, 10U>(this, &keyToken, szDefault);
	}

	[[nodiscard]] const wchar_t* GetWString(const CUtlStringToken& keyToken, const wchar_t* wszDefault = L"") const
	{
		return CallVFunc<const wchar_t*, 11U>(this, &keyToken, wszDefault);
	}

	[[nodiscard]] const void* GetPtr(const CUtlStringToken& keyToken) const
	{
		return CallVFunc<const void*, 12U>(this, &keyToken);
	}

	int GetPlayerIndex(const CUtlStringToken& keyToken)
	{
		//   @ida: client.dll -> U8["4C 8B 70 ? 4C 89 7C 24" + 0x3] / sizeof(std::uintptr_t)   @XREF: "attacker", "assister", "userid"
		int nOutIndex;
		CallVFunc<void, 15U>(this, &nOutIndex, &keyToken);
		return nOutIndex;
	}


	// xref: client.dll & 4C 8B A8 80 00 00 00 ("killer")
	inline CBasePlayerController* get_player_controller(const std::string_view token_name) noexcept {
		CUtlStringToken token(token_name.data());
		return CallVFunc<CBasePlayerController*, 16U>(this, &token);

	}
	C_CSPlayerPawn* GetPlayerPawn(const CUtlStringToken& keyToken)
	{
		//   @XREF: "_pawn"
		//   @ida CGameEvent::GetPlayerPawn(const char*): server.dll -> ABS["E8 ? ? ? ? 48 85 C0 74 0B 48 8B C8 E8 ? ? ? ? 4C 8B F0 41 8B 46 08" + 0x1]   @XREF: "userid"
		return CallVFunc<C_CSPlayerPawn*, 17U>(this, &keyToken);
	}

	void SetBool(const CUtlStringToken& keyToken, const bool bValue)
	{
		//   @ida CGameEvent::SetBool(const char*, bool): server.dll -> ABS["E8 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 C0 48 8B D3 48 8B 01 FF 50 38 48 8B 46 10" + 0x1]   @XREF: "canbuy"
		CallVFunc<void, 20U>(this, &keyToken, bValue);
	}

	void SetInt(const CUtlStringToken& keyToken, const int nValue)
	{
		//   @ida: server.dll -> ["48 8D 4D A7 4C 8B B0 ? ? ? ? FF" + 0x7] / sizeof(std::uintptr_t)
		CallVFunc<void, 21U>(this, &keyToken, nValue);
	}

	void SetUint64(const CUtlStringToken& keyToken, const std::uint64_t ullValue)
	{
		CallVFunc<void, 22U>(this, &keyToken, ullValue);
	}

	void SetFloat(const CUtlStringToken& keyToken, const float flValue)
	{
		//   @ida: server.dll -> ["48 8B B0 ? ? ? ? 33 D2 44 89 6C 24" + 0x3] / sizeof(std::uintptr_t)   @XREF: "inferno_expire"
		CallVFunc<void, 23U>(this, &keyToken, flValue);
	}

	void SetString(const CUtlStringToken& keyToken, const char* szValue)
	{
		//   @ida: server.dll -> ["48 8D 4D A7 48 8B B8 ? ? ? ? 33" + 0x7] / sizeof(std::uintptr_t)   @XREF: "weapon"
		CallVFunc<void, 24U>(this, &keyToken, szValue);
	}

	void SetWString(const CUtlStringToken& keyToken, const wchar_t* wszValue)
	{
		CallVFunc<void, 25U>(this, &keyToken, wszValue);
	}

	void SetPtr(const CUtlStringToken& keyToken, const void* pValue)
	{
		CallVFunc<void, 26U>(this, &keyToken, pValue);
	}

	void SetPlayerIndex(const CUtlStringToken& keyToken, const int nIndex)
	{
		//   @ida: server.dll -> ["48 8B B8 ? ? ? ? E8 ? ? ? ? 48 8D 05 ? ? ? ? 44 8B C3 48 8D 55 97 48 89 45 9F 48 8B CE FF D7 48 8D 15" + 0x3] / sizeof(std::uintptr_t)   @XREF: "attacker_pawn"
		CallVFunc<void, 27U>(this, &keyToken, nIndex);
	}

	void SetPlayerController(const CUtlStringToken& keyToken, CBasePlayerController* pPlayerController)
	{
		CallVFunc<void, 28U>(this, &keyToken, pPlayerController);
	}

	void SetPlayerPawn(const CUtlStringToken& keyToken, C_CSPlayerPawn* pPlayerPawn)
	{
		//   @XREF: "_pawn"
		CallVFunc<void, 29U>(this, &keyToken, pPlayerPawn);
	}

public:
	std::byte pad0[0x60]; // 0x08

	void FireEvent(IGameEvent* event);

	bool initialize();
	bool release();
	void* get_player_pawn_from_id(std::string_view event_name) noexcept;

	/* output (name | int):
		team: 2
		dmg_health: 21
		entityid: 316
		defindex: 49
		radius: 1100
	*/
	std::int64_t get_int(std::string_view event_name) noexcept;

	void* get_pointer_from_id(const std::string_view event_name) noexcept;
};
static_assert(sizeof(IGameEvent) == 0x68);



    // todo: you can use this instead of hooking like you did it in csgo (input ur listeners), but that's a task for the reader.
    // client.dll; 48 89 05 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8D 8B E0 00 00 00
    struct event_manager_t {
        void init_event( /*C_CSPlayerPawnBase*/ void* player_pawn_base,
            const std::string_view event_name, void* unk = nullptr) noexcept {
            // (*(*g_pGameEventManager + 24i64))(g_pGameEventManager, a1 + 4680, "round_end", 0i64);
            return MEM::CallVFunc< void, 6U>(this, player_pawn_base, event_name.data(), unk);
        }
    };

	class IGameEventManager2 
	{
	public:
		enum
		{
			SERVERSIDE = 0, // this is a server side listener, event logger etc
			CLIENTSIDE, // this is a client side listenet, HUD element etc
			CLIENTSTUB, // this is a serverside stub for a remote client listener (used by engine only)
			SERVERSIDE_OLD, // legacy support for old server event listeners
			CLIENTSIDE_OLD // legacy support for old client event listeners
		};

		virtual ~IGameEventManager2() { }

		/// load game event descriptions from a file e.g. "resource\gameevents.res"
		///  @Returns: count of loaded game events
		int LoadEventsFromFile(const char* szFileName)
		{
			return CallVFunc<int, 1U>(this, szFileName);
		}

		// remove all and anything
		void Reset()
		{
			CallVFunc<void, 2U>(this);
		}

		
	};

	