#pragma once

template <typename T>
struct RepeatedPtrField_t
{
	struct Rep_t
	{
		int m_nAllocatedSize; // 0x0
		T* m_tElements[(std::numeric_limits<int>::max() - 2 * sizeof(int)) / sizeof(void*)]; // 0x8
	};

	uint64_t unk_field_;
	int m_nCurrentSize; // 0x18
	int m_nTotalSize; // 0x1C
	Rep_t* m_pRep; // 0x20

	template<class T>
	inline T* add(T* element)
	{
		static const auto sub_C92EF0 = MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 48 8B FA 48 8B 49 ? 48 85 C9 74 ? 8B 01"));
		return reinterpret_cast<T * (__fastcall*)(RepeatedPtrField_t*, T*)>(sub_C92EF0)(this, element);
	}

	inline uint64_t unk_field() {
		return unk_field_;
	}

	inline int& size() {
		return m_nCurrentSize;
	}

	inline int& max_size() {
		return m_pRep->m_nAllocatedSize;
	}

	inline T*& operator[](int i) {
		return m_pRep->m_tElements[i];
	}

	inline operator bool() {
		return m_pRep != nullptr;
	}
};

// @source: master/game/shared/in_buttons.h
enum ECommandButtons : int
{
	IN_ATTACK = (1 << 0),
	IN_JUMP = (1 << 1),
	IN_DUCK = (1 << 2),
	IN_FORWARD = (1 << 3),
	IN_BACK = (1 << 4),
	IN_USE = (1 << 5),
	IN_CANCEL = (1 << 6),
	IN_LEFT = (1 << 7),
	IN_RIGHT = (1 << 8),
	IN_MOVELEFT = (1 << 9),
	IN_MOVERIGHT = (1 << 10),
	IN_SECOND_ATTACK = (1 << 11),
	IN_RUN = (1 << 12),
	IN_RELOAD = (1 << 13),
	IN_LEFT_ALT = (1 << 14),
	IN_RIGHT_ALT = (1 << 15),
	IN_SCORE = (1 << 16),
	IN_SPEED = (1 << 17),
	IN_WALK = (1 << 18),
	IN_ZOOM = (1 << 19),
	IN_FIRST_WEAPON = (1 << 20),
	IN_SECOND_WEAPON = (1 << 21),
	IN_BULLRUSH = (1 << 22),
	IN_FIRST_GRENADE = (1 << 23),
	IN_SECOND_GRENADE = (1 << 24),
	IN_MIDDLE_ATTACK = (1 << 25),
	IN_USE_OR_RELOAD = (1 << 26)
};

class CBasePB
{
public:
	void* pVTable; // 0x0
	std::uint32_t m_nHasBits; // 0x8
	std::uint64_t m_nCachedBits; // 0xC
};
static_assert(sizeof(CBasePB) == 0x18);

class CMsgQAngle : public CBasePB
{
public:
	QAngle_t m_angValue; // 0x18
};

class CMsgVector : public CBasePB
{
public:
	Vector4D_t m_vecValue; // 0x18
};

class CCSGOInterpolationInfoPB : public CBasePB
{
public:
	float m_flFraction; // 0x18
	int m_nSrcTick; // 0x1C
	int m_nDstTick; // 0x20
};

class CCSGOInputHistoryEntryPB : public CBasePB
{
public:
	CMsgQAngle* m_pViewCmd; // 0x18
	CMsgVector* m_pShootOriginCmd; // 0x20
	CMsgVector* m_pTargetHeadOriginCmd; // 0x28
	CMsgVector* m_pTargetAbsOriginCmd; // 0x30
	CMsgQAngle* m_pTargetViewCmd; // 0x38
	CCSGOInterpolationInfoPB* m_cl_interp; // 0x40
	CCSGOInterpolationInfoPB* m_sv_interp0; // 0x48
	CCSGOInterpolationInfoPB* m_sv_interp1; // 0x50
	CCSGOInterpolationInfoPB* m_player_interp; // 0x58
	int m_nRenderTickCount; // 0x60
	float m_flRenderTickFraction; // 0x64
	int m_nPlayerTickCount; // 0x68
	float m_flPlayerTickFraction; // 0x6C
	int m_nFrameNumber; // 0x70
	int m_nTargetEntIndex; // 0x74
};

struct CInButtonStatePB : CBasePB
{
	uint64_t m_nValue; // 0x18
	uint64_t m_nValueChanged; // 0x20
	uint64_t m_nValueScroll; // 0x28
};
static_assert(sizeof(CInButtonStatePB) == 0x30);

struct CSubtickMoveStep : CBasePB
{
	uint64_t nButton; // 0x18
	bool bPressed; // 0x20
	float flWhen; // 0x24
	float flAnalogForwardDelta; // 0x28
	float flAnalogLeftDelta; // 0x2C
};


class CBaseUserCmdPB : public CBasePB
{
public:
	RepeatedPtrField_t<CSubtickMoveStep> m_subtickMovesField; // 0x18
	const char* m_szMoveCrc; // 0x20
	CInButtonStatePB* m_pInButtonState; // 0x28
	CMsgQAngle* m_pViewangles; // 0x30
	int32_t m_nCommandNumber; // 0x38
	int32_t m_nTickCount; // 0x3C
	float m_flForwardMove; // 0x40
	float m_flSideMove; // 0x44
	float m_flUpMove; // 0x48
	int32_t m_nImpulse; // 0x4C
	int32_t m_nWeaponSelect; // 0x50
	int32_t m_nRandomSeed; // 0x54
	int32_t m_nMousedX; // 0x58
	int32_t m_nMousedY; // 0x5C
	uint32_t m_nConsumedServerAngleChanges; // 0x60
	int m_nCmdFlags; // 0x64
	uint32_t m_nPawnEntityHandle; // 0x68
	char pad_007D[3]; //0x007D

	CSubtickMoveStep* AddSubTickMove();
};

class CCSGOUserCmdPB
{
public:
	std::uint32_t m_nHasBits; // 0x0
	std::uint64_t m_nCachedSize; // 0x8
	RepeatedPtrField_t<CCSGOInputHistoryEntryPB> m_inputHistoryField; // 0x10
	CBaseUserCmdPB* m_pBaseCmd; // 0x18
	int32_t m_nAttack3StartHhistoryIndex; // 0x20
	int32_t m_nAttack1StartHhistoryIndex; // 0x24
	int32_t m_nAttack2StartHhistoryIndex; // 0x28
};


struct CInButtonState
{
	void* pVTable; // 0x0
	uint64_t m_nValue; // 0x8
	uint64_t m_nValueChanged; // 0x10
	uint64_t m_nValueScroll; // 0x18
};
static_assert(sizeof(CInButtonStatePB) == 0x30);

class CUserCmd
{
public:
	void* pVTable; // 0x0
	CCSGOUserCmdPB m_csgoUserCmd; // 0x20
	CInButtonState m_nButtons; // 0x30
	MEM_PAD(0x20); // 0x48
	CSubtickMoveStep* GetSubTickMoveStep(int nIndex)
	{
		if (nIndex >= m_csgoUserCmd.m_pBaseCmd->m_subtickMovesField.m_pRep->m_nAllocatedSize)
			return nullptr;

		return m_csgoUserCmd.m_pBaseCmd->m_subtickMovesField.m_pRep->m_tElements[nIndex];
	}
	CCSGOInputHistoryEntryPB* GetInputHistoryEntry(int nIndex)
	{
		if (nIndex >= m_csgoUserCmd.m_inputHistoryField.m_pRep->m_nAllocatedSize)
			return nullptr;

		return m_csgoUserCmd.m_inputHistoryField.m_pRep->m_tElements[nIndex];
	}
	void SetPlayerTickCount() {
		for (int i = 0; i < this->m_csgoUserCmd.m_inputHistoryField.m_pRep->m_nAllocatedSize; i++)
		{
			CCSGOInputHistoryEntryPB* pInputEntry = this->GetInputHistoryEntry(i);
			if (pInputEntry == nullptr)
				continue;

			if (pInputEntry->m_pViewCmd == nullptr)
				continue;

			pInputEntry->m_nPlayerTickCount = 0;
		}
	}
	void SetSubTickAngle(const QAngle_t& angView)
	{
		for (int i = 0; i < this->m_csgoUserCmd.m_inputHistoryField.m_pRep->m_nAllocatedSize; i++)
		{
			CCSGOInputHistoryEntryPB* pInputEntry = this->GetInputHistoryEntry(i);
			if (pInputEntry == nullptr)
				continue;

			if (pInputEntry->m_pViewCmd == nullptr)
				continue;

			pInputEntry->m_pViewCmd->m_angValue = angView;
		}
	}
	void AdjustAttackStartIndex(int nTick) {
		this->m_csgoUserCmd.m_nAttack1StartHhistoryIndex = nTick;
		this->m_csgoUserCmd.m_nAttack2StartHhistoryIndex = nTick;
		this->m_csgoUserCmd.m_nAttack3StartHhistoryIndex = nTick;
	}
};
static_assert(sizeof(CUserCmd) == 0x88);