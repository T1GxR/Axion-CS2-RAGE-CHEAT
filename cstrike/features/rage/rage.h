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
class CCSGOInput;
class IGameEvent;
class C_CSWeaponBase;
class CCSWeaponBaseVData;
class scan_data;
class scan_point;
class scanned_target;
#define MAX_STUDIO_BONES 1024

#include <array>

namespace F::RAGE
{
	enum events {
		round_start = 1,
		player_death = 2,
	};


	class impl {
	public:


		enum reset {
			entity,
			aimbot

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


		enum stop_mode {
			slow = 0,
			early = 1
		};

		enum silent_info {
			skipping = 0,
			on_tick = 1,
		};

		struct aim_info {
			int minimum_damage;
			int rage_hitchance;
			int rage_target_select;
			int rage_enable;
			bool auto_stop;
			bool penetration;
			bool remove_weapon_accuracy_spread;
			bool hitbox_head;
			bool hitbox_neck;
			bool hitbox_uppeer_chest;
			bool hitbox_chest;
			bool hitbox_stomach;
			bool hitbox_legs;
			bool hitbox_feets;
			bool rapid_fire;
			bool failed_hitchance = false;
			bool do_rapid_fire;
			bool accurate;
		}rage_data;

		enum command_msg {
			none = 0,
			rapid_fire = 1,
			teleport = 2,
			silent = 3,
		};
		enum response_msg {
			empty = 0,
			validated_view_angles = 1,
		};

		struct SubTickData {
			QAngle_t best_point = QAngle_t(0, 0, 0);
			Vector_t best_point_vec = Vector_t(0, 0, 0);
			command_msg command = command_msg::none;
			response_msg response = response_msg::empty;

			void reset() {
				best_point.Reset();
				best_point_vec = Vector_t(0, 0, 0);
				command = command_msg::none;
				response = response_msg::empty;
			}

		}sub_tick_data;

		float GetSpread(C_CSWeaponBase* weapon);
		float GetInaccuracy(C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon);
		float HitchanceFast(C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon);
		void Hitchance(C_CSPlayerPawn* pLocal, C_CSPlayerPawn* record, C_CSWeaponBase* weapon, QAngle_t vAimpoint);
		void AutomaticStop(C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, CUserCmd* cmd, stop_mode mode = slow);
		void SortTarget();
		/* void inits */
		bool valid(C_CSPlayerPawn* pawn, C_CSPlayerPawn* pLocal, bool check = false);
		void ScanPoint(C_CSPlayerPawn* pLocal, CUserCmd* cmd, C_CSPlayerPawn* record, scan_data& data, const Vector_t& shoot_pos, bool optimized);
		std::vector <scan_point> get_points(C_CSPlayerPawn* pLocal, C_CSPlayerPawn* record, uint32_t hitbox, bool from_aim);
		void Scan();
		void Triggerbot(CUserCmd* cmd, C_BaseEntity* localent, C_BaseEntity* playerent, C_CSPlayerPawn* local, C_CSPlayerPawn* player, CCSWeaponBaseVData* vdata);
		void BuildSeed();
		void SetupTarget(C_CSPlayerPawn* pawn);
		void SetupAdaptiveWeapon(C_CSPlayerPawn* pawn);
		void ScanTarget(C_CSPlayerPawn* pLocal, CUserCmd* cmd, QAngle_t viewangles);
		Vector_t eye_pos;
		float ScaleDamage(C_CSPlayerPawn* target, C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, Vector_t aim_point, float& dmg, bool& canHit);
		bool CanHit(Vector_t start, Vector_t end, C_CSPlayerPawn* pLocal, C_CSPlayerPawn* record, int box);
		void Reset(reset type);
		void Run(C_CSPlayerPawn* pLocal, CCSGOInput* pInput, CUserCmd* cmd);
		bool Ready(C_CSPlayerPawn* pLocal);
		void Events(IGameEvent* event_listener, events type);
		bool IsAccurate(scan_data& data, CUserCmd* cmd, C_CSPlayerPawn* local);
		bool Attack(CUserCmd* cmd, C_CSPlayerPawn* pLocal, scan_data& data);
		void AjustRecoil(C_CSPlayerPawn* player, CUserCmd* pCmd);
		/* init class */
		aim_info aimbot_info;
		silent_info silent_info;
		inline static std::vector<std::pair<float, float>> m_computed_seeds;

	};
	const auto rage = std::make_unique<impl>();
}

namespace F::LAGCOMP
{

	class impl {
	public:
		/* void inits */
		void Initialize();
		void Start(CUserCmd* cmd) noexcept;
		void Render() noexcept;
	};
	const auto lagcomp = std::make_unique<impl>();
}
