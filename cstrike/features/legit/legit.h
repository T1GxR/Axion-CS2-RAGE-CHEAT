#pragma once

#include "../../common.h"
#include <memory>
// used: draw system
#include "../../utilities/draw.h"
#include "../../sdk/datatypes/vector.h"
#include "../../sdk/datatypes/transform.h"
#include "../../sdk/datatypes/qangle.h"
#include "../cstrike/core/config.h"
class CCSPlayerController;
class C_BaseEntity;
class C_CSPlayerPawn;
class CBaseHandle;
class CEntityInstance;
class CUserCmd;
class CBaseUserCmdPB;
class QAngle_t;
class IGameEvent;
class C_CSWeaponBase;
class CCSWeaponBaseVData;

namespace F::LEGIT
{
	enum events {
		round_start = 1,
		player_death = 2,
	};

	class impl {
	public:
		enum reset {
			entity, 
			aimbot,
			recoil
		};
		enum scan_mode {
			single, /* only 1 hitbox */
			adaptive /* multiple bones pushed in the same time */
		};

		enum hitboxes {
			scan_head,
			scan_neck,
			scan_chest,
			scan_pelvis
		};

		struct aim_info {
			int legit_fov;
			int legit_key;
			int legit_key_style;
			int legit_target_selection;
			bool legit_fov_visualize;
			bool remove_weapon_accuracy_spread;
			int legit_smooth;
			int legit_rcs_shots;
			bool legit_rcs;
			bool PunchRandomization;
			bool legit_rcs_shots_enable;
			float legit_rcs_smoothx;
			float legit_rcs_smoothy;
			bool legit_enable;
			bool legit_visibility_check;
			bool hitbox_head;
			bool hitbox_neck;
			bool hitbox_uppeer_chest;
			bool hitbox_chest;
			bool hitbox_stomach;
			bool hitbox_leg_l;
			bool hitbox_leg_r;
			float punch_y;
			float  punch_x;
			bool trigger_enable;
			int trigger_hc;
	
		}legit_data;

		struct target_info {
			float target_fov;
			Vector_t best_target_pos;
			C_CSPlayerPawn* m_target;
		};

		/* void inits */
		bool valid( C_CSPlayerPawn* pawn, C_CSPlayerPawn* pLocal, bool check = false);
		
		void Scan();
		void Triggerbot(CUserCmd* cmd,  C_BaseEntity* localent, C_BaseEntity* playerent, C_CSPlayerPawn* local, C_CSPlayerPawn* player, CCSWeaponBaseVData* vdata);

		void SetupTarget(C_CSPlayerPawn* pawn);
		void SetupAdaptiveWeapon(C_CSPlayerPawn* pawn);
		void ScanTarget(C_CSPlayerPawn* pLocal, QAngle_t viewangles);
		bool Hitchance(C_CSPlayerPawn* pLocal, C_CSPlayerPawn* ent, C_CSWeaponBase* weapon, QAngle_t vAimpoint, int iChance);

		void Reset(reset type);
		void Run(CUserCmd* cmd);

		bool Ready(C_CSPlayerPawn* pLocal);
		void Events(IGameEvent* event_listener, events type);
		/* init class */
		aim_info aimbot_info;
		target_info target_info;
		C_CSPlayerPawn* pLocal;

	};
	const auto legit = std::make_unique<impl>();
}
