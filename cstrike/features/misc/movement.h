#pragma once

class CUserCmd;
class CBaseUserCmdPB;
class CCSGOInputHistoryEntryPB;
class CBasePlayerController;
class CCSPlayerController;
class C_CSPlayerPawn;
class CSubtickMoveStep;
class QAngle_t;

namespace F::MISC::MOVEMENT
{

	void EdgeBug(CCSPlayerController* controler, C_CSPlayerPawn* localPlayer, CUserCmd* cmd);

	void movment_fix(CUserCmd* pCmd, QAngle_t angle);

	void ProcessMovement(CUserCmd* pCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);

	void BunnyHop(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn);
	void AutoStrafe(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn, int type);

	void MovementCorrection(CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputHistory, const QAngle_t& angDesiredViewPoint);

	// will call MovementCorrection && validate user's angView to avoid untrusted ban
	void ValidateUserCommand(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputHistory);

}
