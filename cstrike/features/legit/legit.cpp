#include "legit.h"
#include "../../core/config.h"
#include "../../core/variables.h"
#include "../../sdk/datatypes/usercmd.h"
#include "../../core/sdk.h"
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/iglobalvars.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/datatypes/qangle.h"
#include "../../sdk/datatypes/vector.h"

#include "../cstrike/sdk/interfaces/ccsgoinput.h"
#include "../cstrike/sdk/interfaces/ienginecvar.h"
#include "../lagcomp/lagcomp.h"
#include "../cstrike/sdk/interfaces/events.h"
#include "../penetration/penetration.h"
#include "../cstrike/sdk/interfaces/itrace.h"
#include "../cstrike/core/spoofcall/syscall.h"
#include <iostream>
#include <memoryapi.h>
#include <mutex>
#include <array>
#include "../../core/spoofcall/virtualization/VirtualizerSDK64.h"
#include "../../utilities/inputsystem.h"
float lasttargettime = 0.f;
static std::vector <std::uint32_t> e_hitboxes;
static ImDrawList* g_pBackgroundDrawList = nullptr;
float kill_delay = 0.f;

struct LegitTarget {
    C_CSPlayerPawn* player;
    Vector_t finalPoint;
    int hitgroup;
    LegitTarget(C_CSPlayerPawn* player, const QAngle_t& finalAngle, const int hitroup)
        : player(player), finalPoint(finalPoint), hitgroup(hitroup) {}
};


std::vector<LegitTarget> sortedTargets;
std::vector<C_CSPlayerPawn*> validTargets;

float get_next_primary_attack(C_CSPlayerPawn* local) noexcept {
    if (!I::GlobalVars)
        return std::numeric_limits<float>::max();

    const auto tick = local->ActiveWeapon()->m_nNextPrimaryAttackTick();
    const auto ratio = local->ActiveWeapon()->m_nNextPrimaryAttackTick();
    return (tick + ratio) * I::GlobalVars->flIntervalPerTick;
}
void F::LEGIT::impl::SetupTarget(C_CSPlayerPawn* pLocal)
{
    if (!D::pDrawListActive)
        return;

    if (!I::Engine->IsInGame()) return;

    CCSPlayerController* pLocalController = CCSPlayerController::GetLocalPlayerController();
    if (!pLocalController)
        return;

    pLocal = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalController->GetPawnHandle());
    if (!pLocal)
        return;
    VIRTUALIZER_DOLPHIN_BLACK_START
        const std::lock_guard<std::mutex> guard{ g_cachedEntitiesMutex };

    // reset valid targets before increamenting it  
    legit->Reset(reset::entity);

    for (const auto& it : g_cachedEntities) {

        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(it.m_handle);
        if (pEntity == nullptr)
            continue;

        CBaseHandle hEntity = pEntity->GetRefEHandle();
        if (hEntity != it.m_handle) continue;

        switch (it.m_type) {
        case CachedEntity_t::PLAYER_CONTROLLER:
            CCSPlayerController* CPlayer = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(hEntity);
            if (CPlayer == nullptr)
                break;

            C_CSPlayerPawn* player = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(CPlayer->GetPawnHandle());
            if (!player)
                continue;

            if (player->GetHealth() <= 0 || !pLocal->IsOtherEnemy(player) || player->GetGameSceneNode()->IsDormant() || player->m_bGunGameImmunity())
                continue;

            validTargets.push_back(player);

            continue;
        }
    }
    VIRTUALIZER_DOLPHIN_BLACK_END
}
bool F::LEGIT::impl::valid(C_CSPlayerPawn* player, C_CSPlayerPawn* pLocal, bool check) {
    if (!player)
        return false;
    bool a1 = check ? true : player->Visible(pLocal, TRACE_TYPE::AIMBOT, legit_data.legit_visibility_check);
    bool Invalid = player->GetLifeState() == ELifeState::LIFE_DISCARDBODY || player->GetLifeState() == ELifeState::LIFE_DEAD || player->GetLifeState() == ELifeState::LIFE_DYING;
    if (Invalid || player->GetHealth() <= 0 || !a1 || !pLocal->IsOtherEnemy(player) || player->GetGameSceneNode()->IsDormant() || player->m_bGunGameImmunity())
        return false;

    return true;
}

// math calculations after Quarention update and shit
// qangle conversions are a mess here tho
QAngle_t NormalizeAngle(QAngle_t& angle) noexcept {
    angle.Clamp();
    return angle;
}

QAngle_t ToAngle(const Vector_t& vec) noexcept {
    return QAngle_t{
        M_RAD2DEG(std::atan2(-vec.z, std::hypot(vec.x, vec.y))),
        M_RAD2DEG(std::atan2(vec.y, vec.x)),
        0.00f
    }.Clamp();
}

static Vector_t get_target_angle(C_CSPlayerPawn* localplayer, Vector_t position)
{
    Vector_t eye_position = localplayer->GetEyePosition();
    Vector_t angle = position;

    angle.x = position.x - eye_position.x;
    angle.y = position.y - eye_position.y;
    angle.z = position.z - eye_position.z;

    angle.Normalizes();
    MATH::vec_angles(angle, &angle);

    angle.clamp();
    return angle;
}

QAngle_t CalculateRelativeAngle(const Vector_t& source, const Vector_t& dest, const QAngle_t& view_angles) noexcept {
    QAngle_t delta = ToAngle(dest - source);

    delta -= view_angles;

    delta = NormalizeAngle(delta);

    return delta;
}

float DistanceToCrosshair(C_CSPlayerPawn* target, const Vector_t& hitboxPos)
{
    auto center = ImGui::GetIO().DisplaySize * 0.5f;
    ImVec2 out;
    auto screenPos = D::WorldToScreen(hitboxPos, out);

    if (screenPos) {
        ImVec2 crosshairPos = center;
        ImVec2 hitboxScreenPos = out;

        float deltaX = crosshairPos.x - hitboxScreenPos.x;
        float deltaY = crosshairPos.y - hitboxScreenPos.y;

        return std::sqrt(deltaX * deltaX + deltaY * deltaY);
    }

    return FLT_MAX;
}

Vector_t CalculateHitboxAngle(Vector_t source, Vector_t destination, QAngle_t viewAngles) {
    Vector_t delta = source - destination;
    Vector_t angles;

    angles.x = M_RAD2DEG(atanf(delta.z / std::hypotf(delta.x, delta.y))) - viewAngles.x;
    angles.y = M_RAD2DEG(atanf(delta.y / delta.x)) - viewAngles.y;
    angles.z = 0.0f;

    return angles;
}

void F::LEGIT::impl::ScanTarget(C_CSPlayerPawn* pLocal, QAngle_t viewangles)
{
    if (!pLocal)
        return;

    if (!I::Engine->IsConnected() || !I::Engine->IsInGame()) return;

    // Clear sortedTarget before any interaction
    sortedTargets.clear();

    QAngle_t final_ang;

    // Check if there are valid targets
    if (!validTargets.empty()) {

        if (legit_data.legit_target_selection == 0) {
            std::sort(validTargets.begin(), validTargets.end(), [this, pLocal](C_CSPlayerPawn* a, C_CSPlayerPawn* b) {
                // Calculate distances
                float distA_crosshair = DistanceToCrosshair(pLocal, a->GetEyePosition());
                float distB_crosshair = DistanceToCrosshair(pLocal, b->GetEyePosition());

                return (distA_crosshair < distB_crosshair);
                });
        }
        else {
            std::sort(validTargets.begin(), validTargets.end(), [this, pLocal](C_CSPlayerPawn* a, C_CSPlayerPawn* b) {
                float distA_player = pLocal->GetEyePosition().DistTo(a->GetEyePosition());
                float distB_player = pLocal->GetEyePosition().DistTo(b->GetEyePosition());

                return (distA_player < distB_player);
                });
        }
        // Add the closest visible target to sortedTarget
        for (C_CSPlayerPawn* target : validTargets) {
            if (valid(target, pLocal)) {
                sortedTargets.emplace_back(target, final_ang, 0);
                break;
            }
            else {
                sortedTargets.clear();
                continue;
            }
            continue;
        }

        // Scan hitboxes, etc.
        if (!sortedTargets.empty()) {

            auto target = sortedTargets.front().player;
            if (!target) {
                sortedTargets.front().finalPoint.Invalidate();
                sortedTargets.clear();
                return;
            }

            // sanity
            if (!valid(target, pLocal)) {
                sortedTargets.front().finalPoint.Invalidate();
                sortedTargets.clear();
                return;
            }

            int bestIndex = -1;
            float closest_dist = FLT_MAX;
            auto best_fov = legit_data.legit_fov;


            auto eyepos = pLocal->GetEyePosition();
            auto endeyepos = target->GetEyePosition();
            float dist = pLocal->GetEyePosition().DistTo(target->GetEyePosition());

            float hitbox_scale = {};
            Vector_t hitbox_pos = {};
            Vector4D_t hitbox_rot = {};
            Vector_t best_point = {};
            // prefer hitbox closest to crosshair ( since we legit hacking )
            for (std::uint32_t i : e_hitboxes) {

                target->CalculateHitboxData(i, hitbox_pos, hitbox_rot, hitbox_scale);
                float cross_dist = DistanceToCrosshair(target, hitbox_pos);

                if (cross_dist < closest_dist) {
                    closest_dist = cross_dist;
                    bestIndex = i;
                    continue;
                }

                continue;
            }

            if (bestIndex != -1) { // store best aimpoint info
                sortedTargets.front().hitgroup = target->GetHitGroup(bestIndex);
                target->CalculateHitboxData(bestIndex, sortedTargets.front().finalPoint, hitbox_rot, hitbox_scale);
            }
        }
        else
            return;
    }


}
// setup config hitboxes
void F::LEGIT::impl::Scan() {

    /* emplace menu hitboxes which will be used for hitscan*/

    if (legit_data.hitbox_head) {
        e_hitboxes.emplace_back(HEAD);
    }

    if (legit_data.hitbox_chest) {
        e_hitboxes.emplace_back(CHEST);
        e_hitboxes.emplace_back(RIGHT_CHEST);
        e_hitboxes.emplace_back(LEFT_CHEST);
    }

    if (legit_data.hitbox_stomach) {
        e_hitboxes.emplace_back(STOMACH);
        e_hitboxes.emplace_back(CENTER);
        e_hitboxes.emplace_back(PELVIS);
    }

    if (legit_data.hitbox_leg_l) {
        e_hitboxes.emplace_back(L_LEG);
    }

    if (legit_data.hitbox_leg_r) {
        e_hitboxes.emplace_back(R_LEG);
    }

    return;
}
void F::LEGIT::impl::Reset(reset type) {
    switch (type) {
    case reset::entity:
        sortedTargets.clear();
        validTargets.clear();
        break;
    case reset::aimbot:
        e_hitboxes.clear();
        break;
    }
    return;
}
float CalculateFactor(float distance, float maxDistance, float minFactor, float maxFactor)
{
    // Ensure distance is within valid range
    if (distance > maxDistance)
        distance = maxDistance;

    // Calculate the factor based on the distance
    float factor = minFactor + (maxFactor - minFactor) * (1 - distance / maxDistance);

    return factor;
}
QAngle_t PerformSmooth(QAngle_t currentAngles, const QAngle_t& targetAngles, float smoothFactor, C_CSPlayerPawn* pLocal, Vector_t hitboxPos)
{
    QAngle_t smoothedAngles;
    float effectiveSmoothFactor = 0.f;

    if (smoothFactor == 0)
        effectiveSmoothFactor = 1 * 10.0f;
    else
        effectiveSmoothFactor = smoothFactor * 10.0f;

    // Calculate the difference between target and current angles
    QAngle_t angleDiff = targetAngles - currentAngles;

    // Calculate the distance between the crosshair and the final point
    float distance = DistanceToCrosshair(pLocal, hitboxPos);

    // Calculate intelligent acceleration and deceleration factors based on distance
    float accelFactor = CalculateFactor(distance, effectiveSmoothFactor, 0.1f, 1.0f);
    float decelFactor = CalculateFactor(distance, effectiveSmoothFactor, 0.1f, 1.0f);

    // Apply acceleration and deceleration
    smoothedAngles.x = currentAngles.x + (angleDiff.x / effectiveSmoothFactor) * (accelFactor + (1));
    smoothedAngles.y = currentAngles.y + (angleDiff.y / effectiveSmoothFactor) * (accelFactor + (1));

    // Ensure angles are within valid range
    smoothedAngles.Clamp();

    return smoothedAngles;
}

bool F::LEGIT::impl::Ready(C_CSPlayerPawn* pLocal) {
    if (!pLocal)
        return false;

    auto ActiveWeapon = pLocal->ActiveWeapon();
    if (!ActiveWeapon)
        return false;

    auto data = ActiveWeapon->datawep();
    if (!data)
        return false;

    if (ActiveWeapon->clip1() <= 0)
        return false;


    return true;
}

void F::LEGIT::impl::Events(IGameEvent* ev, events type) {
    if (!C_GET(bool, Vars.legit_enable))
        return;

    if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
        return;

    if (!SDK::LocalController)
        return;

    switch (type) {
    case player_death: {
        auto controller = SDK::LocalController;
        if (!controller)
            break;

        const auto event_controller = ev->get_player_controller(CS_XOR("attacker"));
        if (!event_controller)
            return;


        if (event_controller->GetIdentity()->GetIndex() == controller->GetIdentity()->GetIndex()) {
            const std::int64_t value{ ev->get_int(CS_XOR("dmg_health")) };
            auto delay = C_GET(int, Vars.legit_delay_aim_ms) * 50.f;

            kill_delay = I::GlobalVars->nTickCount + delay;
            // reset targets
            sortedTargets.clear();
        }
    }
                     break;
    case round_start: {
        kill_delay = 0;
        legit->Reset(reset::entity);
    }
                    break;
    }
}

void  F::LEGIT::impl::Triggerbot(CUserCmd* cmd, C_BaseEntity* localent, C_BaseEntity* playerent, C_CSPlayerPawn* local, C_CSPlayerPawn* player, CCSWeaponBaseVData* vdata)
{
    VIRTUALIZER_MUTATE_ONLY_START;
    if (!legit_data.trigger_enable)
        return;

    if (!SDK::LocalController)
        return;

    auto weapon = local->ActiveWeapon();
    if (!weapon)
        return;

    auto data = weapon->datawep();
    if (!data)
        return;

    if (!cmd)
        return;

    auto base = cmd->m_csgoUserCmd.m_pBaseCmd;
    if (!base)
        return;

    if (C_GET(bool, Vars.trigger_on_key) && !LI_FN(GetAsyncKeyState).safe()(legit_data.legit_key))
        return;

    bool should_trigger = player->InsideCrosshair(local, base->m_pViewangles->m_angValue, data->m_flRange());

    if (should_trigger) {

        if (cheat->canShot)
            cmd->m_nButtons.m_nValue |= IN_ATTACK;
    }
    //F::AUTOWALL::c_auto_wall::data_t pendata;
    //F::AUTOWALL::g_auto_wall->pen(pendata, vecStart, vecEnd, playerent, localent, local, player, vdata, pen_dmg, valid);
    // check do we can shoot
    VIRTUALIZER_MUTATE_ONLY_END;

}
void adjust_recoil(C_CSPlayerPawn* player, CUserCmd* pCmd) noexcept {
    if (!player || !pCmd) return;

    auto cache = player->m_aimPunchCache();
    static QAngle_t prev = QAngle_t(0.f, 0.f, 0.f);
    auto pred_punch = cache.m_Size < 0x81 ? cache.m_Data[cache.m_Size - 1] * 2.f : player->m_aimPunchAngle() * 2.f;
    auto delta = cache.m_Size > 0 && cache.m_Size < 0x81 ? prev - pred_punch : QAngle_t(0, 0, 0);

    if (cache.m_Size > 0 && cache.m_Size < 0x81) {
        if (delta != QAngle_t(0, 0, 0)) {
            pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue += delta;
            pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.Clamp();
        }
        prev = pred_punch;
    }

}
void F::LEGIT::impl::Run(CUserCmd* cmd) {

    if (!C_GET(bool, Vars.legit_enable))
        return;

    if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
        return;

    if (!SDK::LocalController || !SDK::LocalPawn)
        return;

    if (!cmd)
        return;

    auto pCmd = cmd->m_csgoUserCmd.m_pBaseCmd;
    if (!pCmd)
        return;

    /* store vars */
    pLocal = SDK::LocalPawn;
    if (!pLocal || !pCmd)
        return;

    if (pLocal->GetHealth() <= 0)
        return;

    auto viewangles = pCmd->m_pViewangles->m_angValue;

    // reset hitboxes
    legit->Reset(reset::aimbot);

    legit->SetupAdaptiveWeapon(pLocal);

    // hitbox menu selection 
    legit->Scan();

    // run target selection 
    legit->SetupTarget(pLocal);

    // stop aimbot for now, no targets/hitboxes found
    if (validTargets.empty() || e_hitboxes.empty())
        return;

    legit->ScanTarget(pLocal, viewangles);
    if (sortedTargets.empty() || !sortedTargets.front().finalPoint.IsValid())
        return;

    // no ammo or not valid weapon 
    if (!legit->Ready(pLocal))
        return;

    auto weapon_data = pLocal->ActiveWeapon();
    if (!weapon_data)
        return;

    auto vdata = weapon_data->datawep();
    if (!vdata)
        return;

    // check if grabbed ent valid & pass this to the penetration sys
    auto entity = I::GameResourceService->pGameEntitySystem->Get(sortedTargets.front().player->GetRefEHandle());
    if (!entity)
        return;

    auto localent = I::GameResourceService->pGameEntitySystem->Get(pLocal->GetRefEHandle());
    if (!localent)
        return;

    // start angle calculation
    auto vec = get_target_angle(pLocal, sortedTargets.front().finalPoint);
    vec.clamp();
    QAngle_t angle; angle.ToVec3(vec);

    // legit conditions
    if (C_GET(unsigned int, Vars.legit_conditions) & LEGIT_DELAY_SHOT && kill_delay >= I::GlobalVars->nTickCount)
        return;

    if (C_GET(unsigned int, Vars.legit_conditions) & LEGIT_IN_AIR && (!pLocal->IsValidMoveType() || !(pLocal->GetFlags() & FL_ONGROUND)))
        return;

    if (C_GET(unsigned int, Vars.legit_conditions) & LEGIT_FLASHED && pLocal->GetFlashDuration() > 0.f) // flashed
        return;

    //TODO:
    //fix autowall damge when visible
    // 
    // store aimbot data
    auto rcs_shots = legit_data.legit_rcs_shots_enable ? legit_data.legit_rcs_shots : 0;
    int smooth = legit_data.legit_smooth;
    auto fov = std::hypotf(viewangles.x - vec.x, viewangles.y - vec.y); //std::hypotf(viewangles->angValue.x - finalAngle.x, viewangles->angValue.y - finalAngle.y); //GetFov(a1, a2);
    const float best_fov = (float)legit_data.legit_fov;
    float server_time = (float)SDK::LocalController->m_nTickBase() * 0.015;
    bool can_shoot = (pLocal->ActiveWeapon()->m_flNextPrimaryAttackTickRatio() <= server_time);
    float pen_dmg = 0.f;
    bool can_hit = false;
    // run penetration system:
    // F::AUTOWALL::c_auto_wall::data_t data;
   // F::AUTOWALL::g_auto_wall->pen(data, vecStart, vecEnd, entity, localent, pLocal, sortedTargets.front().player, vdata, pen_dmg, can_hit);

    legit->Triggerbot(cmd, localent, entity, pLocal, sortedTargets.front().player, vdata);

    if (fov > best_fov)
        return;

    // calculate aimpunch & compensate it 
    static auto prev = QAngle_t(0.f, 0.f, 0.f);
    auto cache = pLocal->m_aimPunchCache();
    auto pred_punch = cache.m_Data[cache.m_Size - 1];
    auto delta = prev - pred_punch * 2.f;
    if (cache.m_Size > 0 && cache.m_Size <= 0xFFFF) {
        pred_punch = cache.m_Data[cache.m_Size - 1];
        prev = pred_punch;
    }
    // maybe peform some delta randomizations
    if (legit_data.PunchRandomization) {
        delta.x *= MATH::fnRandomFloat(0.3f, 0.7f);
        delta.y *= MATH::fnRandomFloat(0.3f, 0.7f);
    }
    // store best_point with compensated aimpunch
    auto best_point = angle + delta * 2.f;
    if (LI_FN(GetAsyncKeyState).safe()(legit_data.legit_key)) {
        // psilent 
        if (C_GET(bool, Vars.legit_silent_aim) || (C_GET(bool, Vars.legit_no_scope) && vdata->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE && !pLocal->IsScoped())) {
        }
        else {
            QAngle_t smoothedAnglesRCS = PerformSmooth(viewangles, best_point, static_cast<float>(smooth), pLocal, vec);
            if (smooth == 0) {
                viewangles = best_point;
                viewangles.Clamp();

            }
            else {
                viewangles = smoothedAnglesRCS;
                viewangles.Clamp();
            }
        }
    }


}

void F::LEGIT::impl::SetupAdaptiveWeapon(C_CSPlayerPawn* pLocal) {
    if (!pLocal)
        return;

    if (!C_GET(bool, Vars.legit_enable))
        return;

    auto ActiveWeapon = pLocal->ActiveWeapon();
    if (!ActiveWeapon)
        return;

    auto data = ActiveWeapon->datawep();
    if (!data)
        return;

    switch (data->m_WeaponType()) {

    case WEAPONTYPE_PISTOL:
        legit_data.trigger_enable = C_GET(bool, Vars.trigger_enable_p);
        legit_data.trigger_hc = C_GET(int, Vars.trigger_hitchance_p);

        legit_data.legit_fov = C_GET(int, Vars.legit_fov_pistol);
        legit_data.legit_key = C_GET(KeyBind_t, Vars.legit_key_pistol).uKey;
        legit_data.legit_key_style = 0;  // You may need to set the appropriate value
        legit_data.legit_target_selection = C_GET(int, Vars.legit_target_selection);
        legit_data.legit_fov_visualize = C_GET(bool, Vars.legit_fov_visualize);
        legit_data.remove_weapon_accuracy_spread = C_GET(bool, Vars.remove_weapon_accuracy_spread);
        legit_data.legit_smooth = C_GET(int, Vars.legit_smooth_pistol);
        legit_data.legit_rcs_shots = C_GET(int, Vars.legit_rcs_shots_pistol);
        legit_data.legit_rcs = C_GET(bool, Vars.legit_rcs_pistol);
        legit_data.PunchRandomization = C_GET(bool, Vars.PunchRandomization_pistol);
        legit_data.legit_rcs_shots_enable = C_GET(bool, Vars.legit_rcs_shots_enable_pistol);
        legit_data.legit_rcs_smoothx = C_GET(float, Vars.legit_rcs_smoothx_pistol);
        legit_data.legit_rcs_smoothy = C_GET(float, Vars.legit_rcs_smoothy_pistol);
        legit_data.legit_visibility_check = C_GET(bool, Vars.legit_visibility_check_pistol);
        legit_data.hitbox_head = C_GET(bool, Vars.hitbox_head_pistol);
        legit_data.hitbox_neck = C_GET(bool, Vars.hitbox_neck_pistol);
        legit_data.hitbox_uppeer_chest = C_GET(bool, Vars.hitbox_uppeer_chest_pistol);
        legit_data.hitbox_chest = C_GET(bool, Vars.hitbox_chest_pistol);
        legit_data.hitbox_stomach = C_GET(bool, Vars.hitbox_stomach_pistol);
        legit_data.hitbox_leg_l = C_GET(bool, Vars.hitbox_leg_l_pistol);
        legit_data.hitbox_leg_r = C_GET(bool, Vars.hitbox_leg_r_pistol);
        legit_data.punch_x = C_GET(bool, Vars.punch_x_pistol);
        legit_data.punch_y = C_GET(bool, Vars.punch_y_pistol);
        break;

    case WEAPONTYPE_MACHINEGUN:
        legit_data.trigger_enable = C_GET(bool, Vars.trigger_enable_m);
        legit_data.trigger_hc = C_GET(int, Vars.trigger_hitchance_m);
        legit_data.legit_fov = C_GET(int, Vars.legit_fov_machinegun);
        legit_data.legit_key = C_GET(KeyBind_t, Vars.legit_key_machinegun).uKey;
        legit_data.legit_key_style = 0;  // Set the appropriate value
        legit_data.legit_target_selection = C_GET(int, Vars.legit_target_selection_machinegun);
        legit_data.legit_fov_visualize = C_GET(bool, Vars.legit_fov_visualize);
        legit_data.remove_weapon_accuracy_spread = C_GET(bool, Vars.remove_weapon_accuracy_spread);
        legit_data.legit_smooth = C_GET(int, Vars.legit_smooth_machinegun);
        legit_data.legit_rcs_shots = C_GET(int, Vars.legit_rcs_shots_machinegun);
        legit_data.legit_rcs = C_GET(bool, Vars.legit_rcs_machinegun);
        legit_data.PunchRandomization = C_GET(bool, Vars.PunchRandomization_machinegun);
        legit_data.legit_rcs_shots_enable = C_GET(bool, Vars.legit_rcs_shots_enable_machinegun);
        legit_data.legit_rcs_smoothx = C_GET(float, Vars.legit_rcs_smoothx_machinegun);
        legit_data.legit_rcs_smoothy = C_GET(float, Vars.legit_rcs_smoothy_machinegun);
        legit_data.legit_visibility_check = C_GET(bool, Vars.legit_visibility_check_machinegun);
        legit_data.hitbox_head = C_GET(bool, Vars.hitbox_head_machinegun);
        legit_data.hitbox_neck = C_GET(bool, Vars.hitbox_neck_machinegun);
        legit_data.hitbox_uppeer_chest = C_GET(bool, Vars.hitbox_uppeer_chest_machinegun);
        legit_data.hitbox_chest = C_GET(bool, Vars.hitbox_chest_machinegun);
        legit_data.hitbox_stomach = C_GET(bool, Vars.hitbox_stomach_machinegun);
        legit_data.hitbox_leg_l = C_GET(bool, Vars.hitbox_leg_l_machinegun);
        legit_data.hitbox_leg_r = C_GET(bool, Vars.hitbox_leg_r_machinegun);
        legit_data.punch_x = C_GET(bool, Vars.punch_x_machinegun);
        legit_data.punch_y = C_GET(bool, Vars.punch_y_machinegun);
        break;

    case WEAPONTYPE_RIFLE:
        legit_data.trigger_enable = C_GET(bool, Vars.trigger_enable_a);
        legit_data.trigger_hc = C_GET(int, Vars.trigger_hitchance_a);
        legit_data.legit_fov = C_GET(int, Vars.legit_fov_assultrifles);
        legit_data.legit_key = C_GET(KeyBind_t, Vars.legit_key_assultrifles).uKey;
        legit_data.legit_key_style = 0;  // Set the appropriate value
        legit_data.legit_target_selection = C_GET(int, Vars.legit_target_selection_assultrifles);
        legit_data.legit_fov_visualize = C_GET(bool, Vars.legit_fov_visualize);
        legit_data.remove_weapon_accuracy_spread = C_GET(bool, Vars.remove_weapon_accuracy_spread);
        legit_data.legit_smooth = C_GET(int, Vars.legit_smooth_assultrifles);
        legit_data.legit_rcs_shots = C_GET(int, Vars.legit_rcs_shots_assultrifles);
        legit_data.legit_rcs = C_GET(bool, Vars.legit_rcs_assultrifles);
        legit_data.PunchRandomization = C_GET(bool, Vars.PunchRandomization_assultrifles);
        legit_data.legit_rcs_shots_enable = C_GET(bool, Vars.legit_rcs_shots_enable_assultrifles);
        legit_data.legit_rcs_smoothx = C_GET(float, Vars.legit_rcs_smoothx_assultrifles);
        legit_data.legit_rcs_smoothy = C_GET(float, Vars.legit_rcs_smoothy_assultrifles);
        legit_data.legit_visibility_check = C_GET(bool, Vars.legit_visibility_check_assultrifles);
        legit_data.hitbox_head = C_GET(bool, Vars.hitbox_head_assultrifles);
        legit_data.hitbox_neck = C_GET(bool, Vars.hitbox_neck_assultrifles);
        legit_data.hitbox_uppeer_chest = C_GET(bool, Vars.hitbox_uppeer_chest_assultrifles);
        legit_data.hitbox_chest = C_GET(bool, Vars.hitbox_chest_assultrifles);
        legit_data.hitbox_stomach = C_GET(bool, Vars.hitbox_stomach_assultrifles);
        legit_data.hitbox_leg_l = C_GET(bool, Vars.hitbox_leg_l_assultrifles);
        legit_data.hitbox_leg_r = C_GET(bool, Vars.hitbox_leg_r_assultrifles);
        legit_data.punch_x = C_GET(bool, Vars.punch_x_assultrifles);
        legit_data.punch_y = C_GET(bool, Vars.punch_y_assultrifles);
        break;
    case WEAPONTYPE_SNIPER_RIFLE:
        legit_data.trigger_enable = C_GET(bool, Vars.trigger_enable_s);
        legit_data.trigger_hc = C_GET(int, Vars.trigger_hitchance_s);
        legit_data.legit_fov = C_GET(int, Vars.legit_fov_snipers);
        legit_data.legit_key = C_GET(KeyBind_t, Vars.legit_key_snipers).uKey;
        legit_data.legit_key_style = 0;  // Set the appropriate value
        legit_data.legit_target_selection = C_GET(int, Vars.legit_target_selection_snipers);
        legit_data.legit_fov_visualize = C_GET(bool, Vars.legit_fov_visualize);
        legit_data.remove_weapon_accuracy_spread = C_GET(bool, Vars.remove_weapon_accuracy_spread);
        legit_data.legit_smooth = C_GET(int, Vars.legit_smooth_snipers);
        legit_data.legit_rcs_shots = C_GET(int, Vars.legit_rcs_shots_snipers);
        legit_data.legit_rcs = C_GET(bool, Vars.legit_rcs_snipers);
        legit_data.PunchRandomization = C_GET(bool, Vars.PunchRandomization_snipers);
        legit_data.legit_rcs_shots_enable = C_GET(bool, Vars.legit_rcs_shots_enable_snipers);
        legit_data.legit_rcs_smoothx = C_GET(float, Vars.legit_rcs_smoothx_snipers);
        legit_data.legit_rcs_smoothy = C_GET(float, Vars.legit_rcs_smoothy_snipers);
        legit_data.legit_visibility_check = C_GET(bool, Vars.legit_visibility_check_snipers);
        legit_data.hitbox_head = C_GET(bool, Vars.hitbox_head_snipers);
        legit_data.hitbox_neck = C_GET(bool, Vars.hitbox_neck_snipers);
        legit_data.hitbox_uppeer_chest = C_GET(bool, Vars.hitbox_uppeer_chest_snipers);
        legit_data.hitbox_chest = C_GET(bool, Vars.hitbox_chest_snipers);
        legit_data.hitbox_stomach = C_GET(bool, Vars.hitbox_stomach_snipers);
        legit_data.hitbox_leg_l = C_GET(bool, Vars.hitbox_leg_l_snipers);
        legit_data.hitbox_leg_r = C_GET(bool, Vars.hitbox_leg_r_snipers);

        legit_data.punch_x = C_GET(bool, Vars.punch_x_snipers);
        legit_data.punch_y = C_GET(bool, Vars.punch_y_snipers);

        break;
    }

}