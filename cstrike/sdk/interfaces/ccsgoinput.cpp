#include "ccsgoinput.h"
#include "../datatypes/usercmd.h"
#include "../../utilities/memory.h"

CSubtickMoveStep* CBaseUserCmdPB::AddSubTickMove()
{
	if (m_subtickMovesField && m_subtickMovesField.size() < m_subtickMovesField.max_size())
		return m_subtickMovesField[m_subtickMovesField.size()++];

	static auto sub_258D30 = MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8B D0 48 8D 4B ? E8 ? ? ? ? 48 8B D0") + 0x1);

	CSubtickMoveStep* subtick = reinterpret_cast<CSubtickMoveStep* (__fastcall*)(uint64_t)>(sub_258D30)(m_subtickMovesField.unk_field());

	m_subtickMovesField.add(subtick);

	return subtick;
}