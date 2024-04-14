#pragma once

// used: mem_pad
#include "../../utilities/memory.h"

// used: cusercmd
#include "../datatypes/usercmd.h"

#define MULTIPLAYER_BACKUP 150

class CCSGOInput
{
public:
	MEM_PAD(0x250);                // Offset 0x0000, Padding
	CUserCmd arrCommands[MULTIPLAYER_BACKUP]; // Offset 0x0250, Array of CUserCmd
	MEM_PAD(0x1);                  // Offset 0x0A50, Padding
	bool bInThirdPerson;           // Offset 0x0A51, Boolean flag
	MEM_PAD(0x22);                 // Offset 0x0A52, Padding
	std::int32_t nSequenceNumber;   // Offset 0x0A74, Integer
	std::int32_t nOldSequenceNumber; // Offset 0x0A78, Integer
	MEM_PAD(0x4);                  // Offset 0x0A7C, Padding
	double dbUnknown;               // Offset 0x0A80, Double
	std::uint64_t nButtonState1;    // Offset 0x0A88, Unsigned 64-bit integer
	std::uint64_t nButtonState2;    // Offset 0x0A90, Unsigned 64-bit integer
	std::uint64_t nButtonState3;    // Offset 0x0A98, Unsigned 64-bit integer
	std::uint64_t nButtonState4;    // Offset 0x0AA0, Unsigned 64-bit integer
	MEM_PAD(0xC);                  // Offset 0x0AA8, Padding
	std::int32_t nMouseDeltaX;      // Offset 0x0AB4, Integer
	std::int32_t nMouseDeltaY;      // Offset 0x0AB8, Integer
	MEM_PAD(0xC);                  // Offset 0x0ABC, Padding
	std::int64_t nOldPressedButton; // Offset 0x0AC8, Integer
	bool bIsButtonPressed;         // Offset 0x0AD0, Boolean flag
	MEM_PAD(0x10F);                // Offset 0x0AD1, Padding
	QAngle_t angViewAngles;         // Offset 0x0BE0, QAngle_t struct
	MEM_PAD(0x8C);                 // Offset 0x0C6C, Padding
	bool bUnknownBool;             // Offset 0x0CF8, Boolean flag


	void InputCamera() {

		using orig = void(CS_FASTCALL*)(void*);
		static auto oInputcam = reinterpret_cast<orig>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 0F 85 8A")));
		CS_ASSERT(oInputcam != nullptr);

		auto backup = *(Vector_t*)((uintptr_t)this + 0x539);
		// store old camera angles

		// call original
		oInputcam(this);



	}
	CUserCmd* GetUserCmd()
	{
		return &arrCommands[nSequenceNumber % MULTIPLAYER_BACKUP];
	}
	QAngle_t GetViewAngle() // 4C 8B C1 85 D2 74 08 
	{
		using fnGetViewAngle = std::int64_t(CS_FASTCALL*)(void*, std::int32_t);
		static auto oGetViewAngle = reinterpret_cast<fnGetViewAngle>(MEM::FindPattern(CLIENT_DLL, CS_XOR("4C 8B C1 85 D2 74 08")));
#ifdef CS_PARANOID
		CS_ASSERT(oGetViewAngle != nullptr);
#endif
		QAngle_t* fn = reinterpret_cast<QAngle_t*>(oGetViewAngle(this, 0));
		return *fn;
	}
	void SetViewAngle(QAngle_t& angView)
	{
		// @ida: this got called before GetMatricesForView
		using fnSetViewAngle = std::int64_t(CS_FASTCALL*)(void*, std::int32_t, QAngle_t&);
		static auto oSetViewAngle = reinterpret_cast<fnSetViewAngle>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 75 3F 48"))); // \xF2\x41\x0F\x10?\x4C\x63\xCA
#ifdef CS_PARANOID
		CS_ASSERT(oSetViewAngle != nullptr);
#endif
		oSetViewAngle(this, 0, std::ref(angView));
	}
};
