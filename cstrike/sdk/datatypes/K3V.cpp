#pragma once
#include "K3V.h"
#include "../../utilities/memory.h"
// sig 1:48 8B C4 55 48 8D A8 08 FD
/*
KeyValues3* KeyValues3::set_type(kv_basic_type a1, kv_basic_type a2)
{
	using fnTypeData = KeyValues3*(__fastcall*)(uint8_t, uint8_t);
	static auto setType = *reinterpret_cast<fnTypeData*>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 55 48 8D A8 08 FD")));
	
#ifdef CS_PARANOID
	CS_ASSERT(setType != nullptr);
#endif

	return setType(a1, a2);
}

//48 89 5C 24 08 48 89 74 24 10 57 48 81 EC E0 00 00 00 48
KeyValues3* KeyValues3::create_object(const char* name, bool unk1)
{

	// Assuming that MEM::FindPattern returns the function address correctly
	using CreateObjectFn = KeyValues3*(__fastcall*)(const char*, bool);
	static CreateObjectFn createObjectFn = *reinterpret_cast<CreateObjectFn*>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 74 24 10 57 48 81 EC E0 00 00 00 48")));
	CS_ASSERT(createObjectFn != nullptr);

	return createObjectFn(name, unk1);
}
*/
/*
//client.dll 48 89 5C 24 ?? 48 89 6C 24 ?? 56 57 41 54 41 56 41 57 48 83 EC 30 0F B6 01 45 0F B6 F9 8B 2A 4D 8B E0 4C 8B 72 08 48 8B F9
void KeyValues3::set_data(const char* name, const char* value, kv_field_type_t field)
{
	using fnSetData = void(__fastcall*)(void*, const char*, const char*, kv_field_type_t);
	static auto setData = reinterpret_cast<fnSetData>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ?? 48 89 6C 24 ?? 56 57 41 54 41 56 41 57 48 83 EC 30 0F B6 01 45 0F B6 F9 8B 2A 4D 8B E0 4C 8B 72 08 48 8B F9")));

#ifdef CS_PARANOID
	CS_ASSERT(setData != nullptr);
#endif

	setData(this, name, value, field);
}

void KeyValues3::set_data(const char* name, uint32_t color)
{
	using fnSetData = void(__fastcall*)(void*, const char*, uint32_t);
	static auto setData = reinterpret_cast<fnSetData>(MEM::FindPattern(PARTICLES_DLL, CS_XOR("E8 ?? ?? ?? ?? 49 C1 E5 21")));

#ifdef CS_PARANOID
	CS_ASSERT(setData != nullptr);
#endif

	setData(this, name,  color);
}

void KeyValues3::set_data(const char* name, int value)
{
	using fnSetData = void(__fastcall*)(void*, const char*, int);
	static auto setData = reinterpret_cast<fnSetData>(MEM::FindPattern(CLIENT_DLL, CS_XOR("")));

#ifdef CS_PARANOID
	CS_ASSERT(setData != nullptr);
#endif

  setData(this, name, value);
}

bool KeyValues3::load_from_buffer()
{
	return false;
}
*/
