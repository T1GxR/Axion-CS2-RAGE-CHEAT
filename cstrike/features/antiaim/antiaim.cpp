#include "antiaim.hpp"
#include <ctime>
#include <cmath>
#include "../../core/variables.h"

void F::ANTIAIM::RunAA(CUserCmd* pCmd)
{
	if (!C_GET(bool, Vars.bAntiAim))
		return;

	if (!I::Engine->IsConnected() || !I::Engine->IsInGame()) // Checking if you are connected and in game
		return;

	if (!SDK::LocalController || !SDK::LocalController->IsPawnAlive()) // Checking if your spectating and alive
		return;

	if (pCmd->m_nButtons.m_nValue & ECommandButtons::IN_USE || pCmd->m_nButtons.m_nValue & ECommandButtons::IN_ATTACK) // Checking if you are not pressing e or attacking
		return;

	if (int32_t nMoveType = SDK::LocalController->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER)
		return;

	C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(SDK::LocalController->GetPawnHandle());

	if (pLocalPawn == nullptr)
		return;

	if (SDK::LocalController->IsThrowingGrenade(SDK::LocalController->GetPlayerWeapon(SDK::LocalPawn)))
		return;

	float flPitch = 0;
	float flYaw = 0;

	switch (C_GET(int, Vars.iBaseYawType))
	{
	case 0:
		break;
	case 1:
		flYaw += -180.f; // Backwards
		break;
	case 2:
		flYaw += 180.f; // Forwards
		break;
	default:
		break;
	}

	switch (C_GET(int, Vars.iPitchType))
	{
	case 0:
		break;
	case 1: // Down
		flPitch = 120.f;
		break;
	case 2: // Up
		flPitch = -120.f;
		break;
	case 3: // Zero
		flPitch = 0.f;
		break;
	default:
		break;
	}

	pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.x = flPitch;
	pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.y += flYaw;

	pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.y = MATH::AngleNormalize(pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.y);
	pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.x = MATH::AngleNormalize(pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.x);
	pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.z = 0.f;
}