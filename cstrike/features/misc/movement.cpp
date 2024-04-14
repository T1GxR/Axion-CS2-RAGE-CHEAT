#include "movement.h"

// used: sdk
#include <iostream>
// used: sdk entity
#include "../../sdk/entity.h"
// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../sdk/datatypes/qangle.h"
// used: convars
#include "../../core/convars.h"
#include "../../sdk/interfaces/ienginecvar.h"
#include "../cstrike/features/enginepred/pred.h"
// used: cheat variables
#include "../../core/variables.h"
#include <array>
#include "../../core/sdk.h"
#include "../../sdk/interfaces/itrace.h"
#include "../cstrike/sdk/interfaces/cgameentitysystem.h"
#include "../antiaim/antiaim.hpp"

struct MovementData {
	QAngle_t angCorrectionView{};

} _move_data;

enum DIR : int {
	FORWARDS = 0,
	BACKWARDS = 180,
	LEFT = 90,
	RIGHT = -90
};

enum MODE : int {
	legit = 0,
	rage = 1
};

void AngleQangles(const QAngle_t& angles, QAngle_t* forward, QAngle_t* right, QAngle_t* up)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	// Convert angles from degrees to radians
	angle = angles.y * (MATH::_PI / 180.0);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles.x * (MATH::_PI / 180.0);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles.z * (MATH::_PI / 180.0);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}

void F::MISC::MOVEMENT::movment_fix(CUserCmd* pCmd, QAngle_t angle)
{
	QAngle_t wish_angle;
	wish_angle = pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue;
	int revers = wish_angle.x > 89.f ? -1 : 1;
	wish_angle.Clamp();

	QAngle_t view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
	auto viewangles = angle;

	AngleQangles(wish_angle, &view_fwd, &view_right, &view_up);
	AngleQangles(viewangles, &cmd_fwd, &cmd_right, &cmd_up);

	const float v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
	const float v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
	const float v12 = sqrtf(view_up.z * view_up.z);

	const Vector_t norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
	const Vector_t norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
	const Vector_t norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

	const float v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
	const float v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
	const float v18 = sqrtf(cmd_up.z * cmd_up.z);

	const Vector_t norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
	const Vector_t norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
	const Vector_t norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

	const float v22 = norm_view_fwd.x * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove;
	const float v26 = norm_view_fwd.y * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove;
	const float v28 = norm_view_fwd.z * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove;
	const float v24 = norm_view_right.x * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove;
	const float v23 = norm_view_right.y * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove;
	const float v25 = norm_view_right.z * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove;
	const float v30 = norm_view_up.x * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flUpMove;
	const float v27 = norm_view_up.z * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flUpMove;
	const float v29 = norm_view_up.y * pCmd->m_csgoUserCmd.m_pBaseCmd->m_flUpMove;

	pCmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25))
		+ (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)))
		+ (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
	pCmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25))
		+ (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)))
		+ (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
	pCmd->m_csgoUserCmd.m_pBaseCmd->m_flUpMove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25))
		+ (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28)))
		+ (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));

	pCmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = revers * ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25)) + (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)));

	pCmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25)) + (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)));

	pCmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = std::clamp(pCmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove, -1.f, 1.f);

	pCmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = std::clamp(pCmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove, -1.f, 1.f);
}

void F::MISC::MOVEMENT::ProcessMovement(CUserCmd* pCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!pLocalController)
		return;

	if (!pLocalPawn)
		return;

	if (pLocalPawn->GetHealth() <= 0)
		return;

	if (pCmd == nullptr)
		return;

	CBaseUserCmdPB* base = pCmd->m_csgoUserCmd.m_pBaseCmd;
	if (base == nullptr)
		return;

	// check if player is in noclip or on ladder or in water
	if (const int32_t nMoveType = pLocalPawn->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER || pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return;

	EdgeBug(pLocalController, pLocalPawn, pCmd);

	if (pLocalPawn->IsValidMoveType()) {
		AutoStrafe(pCmd, base, pLocalPawn, C_GET(int, Vars.bAutostrafeMode));
		BunnyHop(pCmd, base, pLocalPawn);
		movment_fix(pCmd, F::ANTIAIM::angStoredViewBackup);
	}

	for (int i = 0; i < pCmd->m_csgoUserCmd.m_inputHistoryField.m_pRep->m_nAllocatedSize; i++)
	{
		CCSGOInputHistoryEntryPB* pInputEntry = pCmd->GetInputHistoryEntry(i);
		if (pInputEntry == nullptr)
			continue;

		if (pInputEntry->m_pViewCmd == nullptr)
			continue;

		_move_data.angCorrectionView = pInputEntry->m_pViewCmd->m_angValue;

		ValidateUserCommand(pCmd, base, pInputEntry);

	}
}

void rotate_movement(float yaw, const QAngle_t& angles, float& fmove, float& smove) {
	float rotation = M_DEG2RAD(angles.y - yaw);

	float cos_rot = std::cos(rotation);
	float sin_rot = std::sin(rotation);

	float new_forwardmove = (cos_rot * fmove) - (sin_rot * smove);
	float new_sidemove = (sin_rot * fmove) + (cos_rot * smove);

	fmove = new_forwardmove;
	smove = new_sidemove;
}

void F::MISC::MOVEMENT::EdgeBug(CCSPlayerController* controler, C_CSPlayerPawn* localPlayer, CUserCmd* cmd) {
	if (!C_GET(bool, Vars.edge_bug))
		return;

	if (!controler || !localPlayer || localPlayer->GetHealth() <= 0)
		return;

	C_CSPlayerPawn* pred = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(controler->m_hPredictedPawn());
	if (!pred)
		return;

	static bool bhopWasEnabled = true;
	bool JumpDone;

	bool unduck = true;

	float max_radius = MATH::_PI * 2;
	float step = max_radius / 128;
	float xThick = 23;
	bool valid = GetAsyncKeyState(C_GET(KeyBind_t, Vars.edge_bug_key).uKey);
	if (!valid) {
		return;
	}

	if (valid) {
		I::Cvar->Find(FNV1A::Hash(CS_XOR("sv_min_jump_landing_sound")))->value.fl = 63464578.f;
	}
	else {
		I::Cvar->Find(FNV1A::Hash(CS_XOR("sv_min_jump_landing_sound")))->value.fl = 260;
	}

	static bool edgebugging = false;
	static int edgebugging_tick = 0;

	if (!edgebugging) {

		int flags = localPlayer->GetFlags();
		float z_velocity = floor(localPlayer->m_vecVelocity().z);

		for (int i = 0; i < 64; i++) {

			if (z_velocity < -7 && floor(localPlayer->m_vecVelocity().z) == -7 && !(flags & FL_ONGROUND) && localPlayer->GetMoveType() != MOVETYPE_NOCLIP) {
				edgebugging_tick = cmd->m_csgoUserCmd.m_pBaseCmd->m_nTickCount + i;

				edgebugging = true;
				break;
			}
			else {
				z_velocity = floor(localPlayer->m_vecVelocity().z);
				flags = localPlayer->GetFlags();
			}
		}
	}
	else {

		cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = 0.f;
		cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = 0.f;
		cmd->m_csgoUserCmd.m_pBaseCmd->m_flUpMove = 0.f;
		cmd->m_csgoUserCmd.m_pBaseCmd->m_nMousedX = 0.f;
		cmd->m_nButtons.m_nValue |= IN_DUCK;

		if ((localPlayer->GetFlags() & 0x1)) {
			cmd->m_nButtons.m_nValue &= ~IN_JUMP;
		}

		Vector_t pos = localPlayer->GetSceneOrigin();

		for (float a = 0.f; a < max_radius; a += step) {
			Vector_t pt;
			pt.x = (xThick * cos(a)) + pos.x;
			pt.y = (xThick * sin(a)) + pos.y;
			pt.z = pos.z;

			Vector_t pt2 = pt;
			pt2.z -= 8192;
			trace_filter_t filter = {};
			I::Trace->Init(filter, localPlayer, 0x1400B, 3, 7);

			game_trace_t trace = {};
			ray_t ray = {};

			I::Trace->TraceShape(ray, &pt, &pt2, filter, trace);

			if (trace.Fraction != 1.0f && trace.Fraction != 0.0f) {
				JumpDone = true;
				cmd->m_nButtons.m_nValue |= IN_DUCK;
			}
		}


		if (cmd->m_csgoUserCmd.m_pBaseCmd->m_nTickCount > edgebugging_tick) {
			edgebugging = false;
			edgebugging_tick = 0;
		}

	}
	trace_filter_t filter = {};
	I::Trace->Init(filter, localPlayer, 0x1400B, 3, 7);

	Vector_t pos = localPlayer->GetSceneOrigin();
	if (pred->m_bInLanding()) {

		for (float a = 0.f; a < max_radius; a += step) {
			Vector_t pt;
			pt.x = (xThick * cos(a)) + pos.x;
			pt.y = (xThick * sin(a)) + pos.y;
			pt.z = pos.z;

			Vector_t pt2 = pt;
			pt2.z -= 8192;

			game_trace_t trace = {};
			ray_t ray = {};

			I::Trace->TraceShape(ray, &pt, &pt2, filter, trace);

			if (trace.Fraction != 1.0f && trace.Fraction != 0.0f) {
				JumpDone = true;


				cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = 0.f;
				cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = 0.f;
				cmd->m_csgoUserCmd.m_pBaseCmd->m_flUpMove = 0.f;
				cmd->m_csgoUserCmd.m_pBaseCmd->m_nMousedX = 0.f;
				cmd->m_nButtons.m_nValue |= IN_DUCK;

				if ((localPlayer->GetFlags() & 0x1)) {
					cmd->m_nButtons.m_nValue &= ~IN_JUMP;
				}

			}
		}

	}
	if (cmd->m_csgoUserCmd.m_pBaseCmd->m_nTickCount > edgebugging_tick) {
		edgebugging = false;
		edgebugging_tick = 0;
	}
}

void F::MISC::MOVEMENT::AutoStrafe(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn, int type)
{
	static uint64_t last_pressed = 0;
	static uint64_t last_buttons = 0;

	if (!C_GET(bool, Vars.bAutostrafe))
		return;

	auto& cmd = I::Input->arrCommands[I::Input->nSequenceNumber % 150];
	bool strafe_assist = C_GET(bool, Vars.bAutostrafeAssistance);
	const auto current_buttons = cmd.m_nButtons.m_nValue;
	auto yaw = MATH::normalize_yaw(pUserCmd->m_pViewangles->m_angValue.y);

	const auto check_button = [&](const uint64_t button)
	{
		if (current_buttons & button && (!(last_buttons & button) || button & IN_MOVELEFT && !(last_pressed & IN_MOVERIGHT) || button &
			IN_MOVERIGHT && !(last_pressed & IN_MOVELEFT) || button & IN_FORWARD && !(last_pressed & IN_BACK) ||
			button & IN_BACK && !(last_pressed & IN_FORWARD)))
		{
			if (strafe_assist)
			{
				if (button & IN_MOVELEFT)
					last_pressed &= ~IN_MOVERIGHT;
				else if (button & IN_MOVERIGHT)
					last_pressed &= ~IN_MOVELEFT;
				else if (button & IN_FORWARD)
					last_pressed &= ~IN_BACK;
				else if (button & IN_BACK)
					last_pressed &= ~IN_FORWARD;
			}

			last_pressed |= button;
		}
		else if (!(current_buttons & button))
			last_pressed &= ~button;
	};

	check_button(IN_MOVELEFT);
	check_button(IN_MOVERIGHT);
	check_button(IN_FORWARD);
	check_button(IN_BACK);

	last_buttons = current_buttons;

	const auto velocity = pLocalPawn->GetAbsVelocity();
	bool wasdstrafe = C_GET(unsigned int, Vars.bAutostrafeMode) == 0;
	bool viewanglestrafe = C_GET(unsigned int, Vars.bAutostrafeMode) == 1;
	float smoothing = C_GET(float, Vars.autostrafe_smooth);

	/*const auto weapon = pLocalPawn->get_weapon_services_ptr()->get_h_active_weapon().get();
	const auto js = weapon && (cfg.weapon_config.is_scout && cfg.weapon_config.cur.scout_jumpshot && pLocalPawn->get_vec_abs_velocity().length_2d() < 50.f);
	const auto throwing_nade = weapon && weapon->is_grenade() && ticks_to_time(local_player->get_tickbase()) >= weapon->get_throw_time() && weapon->get_throw_time() != 0.f;

	if (js)
		return;*/

	if (pLocalPawn->GetFlags() & FL_ONGROUND)
		return;

	auto rotate_movement = [](CUserCmd& cmd, float target_yaw)
	{		auto pUserCmd = cmd.m_csgoUserCmd.m_pBaseCmd;

	const float rot = M_DEG2RAD(pUserCmd->m_pViewangles->m_angValue.y - target_yaw);

	const float new_forward = std::cos(rot) * pUserCmd->m_flForwardMove - std::sin(rot) * pUserCmd->m_flSideMove;
	const float new_side = std::sin(rot) * pUserCmd->m_flForwardMove + std::cos(rot) * pUserCmd->m_flSideMove;

	cmd.m_nButtons.m_nValue &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);
	pUserCmd->m_flForwardMove = std::clamp(new_forward, -1.f, 1.f);
	pUserCmd->m_flSideMove = std::clamp(new_side * -1.f, -1.f, 1.f);

	if (pUserCmd->m_flForwardMove > 0.f)
		cmd.m_nButtons.m_nValue |= IN_FORWARD;
	else if (pUserCmd->m_flForwardMove < 0.f)
		cmd.m_nButtons.m_nValue |= IN_BACK;

	if (pUserCmd->m_flSideMove > 0.f)
		cmd.m_nButtons.m_nValue |= IN_MOVELEFT;
	else if (pUserCmd->m_flSideMove < 0.f)
		cmd.m_nButtons.m_nValue |= IN_MOVERIGHT;
	};


	if (wasdstrafe)
	{
		auto offset = 0.f;
		if (last_pressed & IN_MOVELEFT)
			offset += 90.f;
		if (last_pressed & IN_MOVERIGHT)
			offset -= 90.f;
		if (last_pressed & IN_FORWARD)
			offset *= 0.5f;
		else if (last_pressed & IN_BACK)
			offset = -offset * 0.5f + 180.f;

		yaw += offset;

		pUserCmd->m_flForwardMove = 0.f;
		pUserCmd->m_flSideMove = 0.f;

		rotate_movement(cmd, MATH::normalize_yaw(yaw));

		if (!viewanglestrafe && offset == 0.f)
			return;
	}

	if (pUserCmd->m_flSideMove != 0.0f || pUserCmd->m_flForwardMove != 0.0f)
		return;

	auto velocity_angle = M_RAD2DEG(std::atan2f(velocity.y, velocity.x));
	if (velocity_angle < 0.0f)
		velocity_angle += 360.0f;

	if (velocity_angle < 0.0f)
		velocity_angle += 360.0f;

	velocity_angle -= floorf(velocity_angle / 360.0f + 0.5f) * 360.0f;

	const auto speed = velocity.Length2D();
	const auto ideal = std::clamp(M_RAD2DEG(std::atan2(15.f, speed)), 0.f, 45.f);

	const auto correct = (100.f - smoothing) * 0.02f * (ideal + ideal);

	pUserCmd->m_flForwardMove = 0.f;
	const auto velocity_delta = MATH::normalize_yaw(yaw - velocity_angle);

	/*if (throwing_nade && fabsf(velocity_delta) <=20.f)
	{
		auto &wish_angle = antiaim::wish_angles[globals::current_cmd->command_number % 150];
		wish_angle.y = math::normalize_yaw(yaw);
		globals::current_cmd->forwardmove = 450.f;

		antiaim::fix_movement(globals::current_cmd);
		return;
	}*/

	if (fabsf(velocity_delta) > 170.f && speed > 80.f || velocity_delta > correct && speed > 80.f)
	{
		yaw = correct + velocity_angle;
		pUserCmd->m_flSideMove = -1.f;
		rotate_movement(cmd, MATH::normalize_yaw(yaw));
		return;
	}
	const bool side_switch = I::Input->nSequenceNumber % 2 == 0;

	if (-correct <= velocity_delta || speed <= 80.f)
	{
		if (side_switch)
		{
			yaw = yaw - ideal;
			pUserCmd->m_flSideMove = -1.f;

		}
		else
		{
			yaw = ideal + yaw;
			pUserCmd->m_flSideMove = 1.f;

		}
		rotate_movement(cmd, MATH::normalize_yaw(yaw));
	}
	else
	{
		yaw = velocity_angle - correct;
		pUserCmd->m_flSideMove = 1.f;

		rotate_movement(cmd, MATH::normalize_yaw(yaw));
	}
}

std::array<bool, 150> onground_tick = {};
std::array<bool, 150> remove_tick = {};
bool pressed_jump = false;

void F::MISC::MOVEMENT::BunnyHop(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn)
{
	bool bWasLastTimeOnGround{};

	pressed_jump = pCmd->m_nButtons.m_nValue & IN_JUMP;

	if (!C_GET(bool, Vars.bAutoBHop) || CONVAR::sv_autobunnyhopping->value.i1)
		return;

	if (pLocalPawn->GetFlags() & FL_ONGROUND)
		pCmd->m_nButtons.m_nValue &= ~IN_JUMP;

	bWasLastTimeOnGround = pLocalPawn->GetFlags() & FL_ONGROUND;

	const float velocity = pLocalPawn->m_vecVelocity().Length2D();

	bool bShouldJump = true;

	if (velocity > 300.f && Vars.nAutoBHopChance < 0.8) bShouldJump = false;

	if (!pLocalPawn->GetFlags() & FL_ONGROUND && !bWasLastTimeOnGround)
		pCmd->m_nButtons.m_nValue &= ~IN_JUMP;

	bWasLastTimeOnGround = pLocalPawn->GetFlags() & FL_ONGROUND;

	/*CSubtickMoveStep* subtick_pressed = pUserCmd->AddSubTickMove();
	subtick_pressed->m_nCachedBits = 7;
	subtick_pressed->nButton = IN_JUMP;
	subtick_pressed->bPressed = true;
	subtick_pressed->flWhen = 0.999f;

	CSubtickMoveStep* subtick_release = pUserCmd->AddSubTickMove();
	subtick_release->m_nCachedBits = 7;
	subtick_release->nButton = IN_JUMP;
	subtick_release->bPressed = false;
	subtick_release->flWhen = 0.999f;*/
}

void F::MISC::MOVEMENT::ValidateUserCommand(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry)
{
	if (pUserCmd == nullptr)
		return;

	// clamp angle to avoid untrusted angle
	if (C_GET(bool, Vars.bAntiUntrusted))
	{
		if (pInputEntry->m_pViewCmd->m_angValue.IsValid())
		{
			pInputEntry->m_pViewCmd->m_angValue.Clamp();
			pInputEntry->m_pViewCmd->m_angValue.z = 0.f;
		}
		else
		{
			pInputEntry->m_pViewCmd->m_angValue = {};
		}
	}


	//MovementCorrection(pUserCmd, pInputEntry, _move_data.angCorrectionView);

	// correct movement buttons while player move have different to buttons values
	// clear all of the move buttons states
	pCmd->m_nButtons.m_nValue &= (~IN_FORWARD | ~IN_BACK | ~IN_LEFT | ~IN_RIGHT);

	// re-store buttons by active forward/side moves
	if (pUserCmd->m_flForwardMove > 0.0f)
		pCmd->m_nButtons.m_nValue |= IN_FORWARD;
	else if (pUserCmd->m_flForwardMove < 0.0f)
		pCmd->m_nButtons.m_nValue |= IN_BACK;

	if (pUserCmd->m_flSideMove > 0.0f)
		pCmd->m_nButtons.m_nValue |= IN_RIGHT;
	else if (pUserCmd->m_flSideMove < 0.0f)
		pCmd->m_nButtons.m_nValue |= IN_LEFT;


	if (!pInputEntry->m_pViewCmd->m_angValue.IsZero())
	{
		float flDeltaX = std::remainderf(pInputEntry->m_pViewCmd->m_angValue.x - _move_data.angCorrectionView.x, 360.f);
		float flDeltaY = std::remainderf(pInputEntry->m_pViewCmd->m_angValue.y - _move_data.angCorrectionView.y, 360.f);


		float flPitch = CONVAR::m_pitch->value.fl;
		float flYaw = CONVAR::m_yaw->value.fl;

		float flSensitivity = CONVAR::sensitivity->value.fl;
		if (flSensitivity == 0.0f)
			flSensitivity = 1.0f;

		pUserCmd->m_nMousedX = static_cast<short>(flDeltaX / (flSensitivity * flPitch));
		pUserCmd->m_nMousedY = static_cast<short>(-flDeltaY / (flSensitivity * flYaw));
	}
}

void F::MISC::MOVEMENT::MovementCorrection(CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry, const QAngle_t& angDesiredViewPoint)
{
	if (pUserCmd == nullptr)
		return;

	Vector_t vecForward = {}, vecRight = {}, vecUp = {};
	angDesiredViewPoint.ToDirections(&vecForward, &vecRight, &vecUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecForward.z = vecRight.z = vecUp.x = vecUp.y = 0.0f;

	vecForward.NormalizeInPlace();
	vecRight.NormalizeInPlace();
	vecUp.NormalizeInPlace();

	Vector_t vecOldForward = {}, vecOldRight = {}, vecOldUp = {};
	pInputEntry->m_pViewCmd->m_angValue.ToDirections(&vecOldForward, &vecOldRight, &vecOldUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecOldForward.z = vecOldRight.z = vecOldUp.x = vecOldUp.y = 0.0f;

	vecOldForward.NormalizeInPlace();
	vecOldRight.NormalizeInPlace();
	vecOldUp.NormalizeInPlace();

	const float flPitchForward = vecForward.x * pUserCmd->m_flForwardMove;
	const float flYawForward = vecForward.y * pUserCmd->m_flForwardMove;
	const float flPitchSide = vecRight.x * pUserCmd->m_flSideMove;
	const float flYawSide = vecRight.y * pUserCmd->m_flSideMove;
	const float flRollUp = vecUp.z * pUserCmd->m_flUpMove;

	// solve corrected movement speed
	pUserCmd->m_flForwardMove = vecOldForward.x * flPitchSide + vecOldForward.y * flYawSide + vecOldForward.x * flPitchForward + vecOldForward.y * flYawForward + vecOldForward.z * flRollUp;
	pUserCmd->m_flSideMove = vecOldRight.x * flPitchSide + vecOldRight.y * flYawSide + vecOldRight.x * flPitchForward + vecOldRight.y * flYawForward + vecOldRight.z * flRollUp;
	pUserCmd->m_flUpMove = vecOldUp.x * flYawSide + vecOldUp.y * flPitchSide + vecOldUp.x * flYawForward + vecOldUp.y * flPitchForward + vecOldUp.z * flRollUp;
}