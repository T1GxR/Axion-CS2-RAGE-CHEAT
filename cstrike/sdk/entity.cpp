#include "entity.h"
// others
#include "../core/hooks.h"
#include "../utilities/draw.h"

// used: convars
#include "../core/convars.h"
#include "interfaces/cgameentitysystem.h"
#include "interfaces/ienginecvar.h"
#include "interfaces/iengineclient.h"

// used: game's definitions, enums
#include "const.h"
#include "../core/csig/sigscan.hpp"
#include "../core/pointer/pointer.hpp"
#include <array>
#include "../features.h"
#include "../cstrike/features/visuals/overlay.h"
#include "../cstrike/features/legit/legit.h"
#include "interfaces/iglobalvars.h"
#include "../cstrike/features/skins/ccsplayerinventory.hpp"
#include "../features/rage/rage.h"

Vector_t C_CSPlayerPawn::GetEyePosition() {
    Vector_t EyePosition;
    MEM::CallVFunc<void, 163>(this, &EyePosition);
    return EyePosition;
}

static CachedEntity_t::Type GetEntityType(C_BaseEntity* pEntity);

static CachedEntity_t::Type GetEntityType(C_BaseEntity* pEntity) {
    if (!pEntity)
        return CachedEntity_t::UNKNOWN;

    SchemaClassInfoData_t* pClassInfo = nullptr;
    pEntity->GetSchemaClassInfo(&pClassInfo);

    if (pClassInfo == nullptr)
        return CachedEntity_t::UNKNOWN;

    const FNV1A_t uHashedName = FNV1A::Hash(pClassInfo->szName);

    if (uHashedName == FNV1A::HashConst(CS_XOR("CCSPlayerController"))) {
        return CachedEntity_t::PLAYER_CONTROLLER;
    }

    return CachedEntity_t::UNKNOWN;
}

// Definition of the global vector and mutex
std::vector<CachedEntity_t> g_cachedEntities;
std::mutex g_cachedEntitiesMutex;
std::unordered_multimap<CachedEntity_t::Type, entity_callbacks> g_Callbacks;

namespace EntCache {

    void CacheCurrentEntities() {
        // This function runs ONCE on injection and caches all the entities if
        // you happen to inject connected on a server.

        if (!I::Engine->IsInGame() || !I::Engine->IsConnected()) return;

        if (!I::GameResourceService->pGameEntitySystem) return;


        int highestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
        for (int i = 1; i <= highestIndex; ++i) {
            C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
            if (!pEntity) continue;

            OnAddEntity(pEntity, pEntity->GetRefEHandle());
        }
    }

    void OnAddEntity(CEntityInstance* inst, CBaseHandle handle) {
        if (!I::GameResourceService->pGameEntitySystem) return;

        C_BaseEntity* pEntity = (C_BaseEntity*)inst;
        if (!pEntity) return;

        // Cache only networked entities.
        // https://developer.valvesoftware.com/wiki/Entity_limit#Source_2_limits
        if (handle.GetEntryIndex() >= 16384) return;
        // call callbacks
        for (auto& [callback_type, callbacks] : g_Callbacks) {
            if (!callbacks.add || callback_type != GetEntityType(pEntity))
                continue;
            callbacks.add(handle.GetEntryIndex(), handle);
        }
        std::lock_guard<std::mutex> lock(g_cachedEntitiesMutex);

        auto it = std::find_if(g_cachedEntities.begin(), g_cachedEntities.end(),
            [handle](const CachedEntity_t& i) {
                return i.m_handle.GetEntryIndex() == handle.GetEntryIndex();
            });

        if (it == g_cachedEntities.end()) {
            CachedEntity_t cachedEntity{};
            cachedEntity.m_handle = handle;
            cachedEntity.m_type = GetEntityType(pEntity);
            cachedEntity.index = handle.GetEntryIndex();
            cachedEntity.reset = false;

            if (cachedEntity.m_type != CachedEntity_t::UNKNOWN) {
                g_cachedEntities.emplace_back(cachedEntity);
            }
        }
        else {
            it->m_handle = handle;
            it->m_type = GetEntityType(pEntity);
        }



    }


    void OnRemoveEntity(CEntityInstance* inst, CBaseHandle handle) {
        if (!I::GameResourceService->pGameEntitySystem) return;

        C_BaseEntity* pEntity = (C_BaseEntity*)inst;
        if (!pEntity) return;

        // call callbacks
        for (auto& [callback_type, callbacks] : g_Callbacks) {
            if (!callbacks.remove)
                continue;

            if (callback_type != GetEntityType(pEntity))
                continue;

            callbacks.remove(handle.GetEntryIndex(), handle);
        }

        std::lock_guard<std::mutex> lock(g_cachedEntitiesMutex);

        auto it = std::find_if(g_cachedEntities.begin(), g_cachedEntities.end(),
            [handle](const CachedEntity_t& i) {
                return i.m_handle == handle;
            });

        if (it == g_cachedEntities.end()) return;

        it->m_draw = false;
        it->m_type = CachedEntity_t::UNKNOWN;

    }
    void RegisterCallback(CachedEntity_t::Type type, entity_callback add, entity_callback remove) noexcept {
        if (!add && !remove)
            return;

        std::unique_lock lock(g_cachedEntitiesMutex);
        g_Callbacks.insert(std::make_pair(type, entity_callbacks{ add, remove }));

        if (!add)
            return;

        // add existing entities
        for (auto& cached_entity : g_cachedEntities) {
            if (cached_entity.m_type == type)
                add(cached_entity.m_handle.GetEntryIndex(), cached_entity.m_handle);
        }
    }


}  // namespace EntCache
// global empty vector for when we can't get the origin
static Vector_t vecEmpty = Vector_t(0, 0, 0);

bool CCSPlayerController::IsThrowingGrenade(C_CSWeaponBase* pBaseWeapon)
{
    if (!pBaseWeapon)
        return false;

    if (!SDK::LocalController->IsPawnAlive() || !SDK::LocalController)
        return false;

    const float flServerTime = TICKS_TO_TIME(this->m_nTickBase());
    const short nDefinitionIndex = pBaseWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex();
    CCSWeaponBaseVData* pWeaponBaseVData = pBaseWeapon->datawep();
    if (!pWeaponBaseVData)
        return false;

    if (pWeaponBaseVData->m_WeaponType() == WEAPONTYPE_GRENADE)
    {
        C_BaseCSGrenade* pGrenade = reinterpret_cast<C_BaseCSGrenade*>(pBaseWeapon);
        if (pGrenade != nullptr)
        {
            return !pGrenade->IsPinPulled() && pGrenade->GetThrowTime() > 0.f && pGrenade->GetThrowTime() < flServerTime;
        }
    }

    return false;
}

CCSPlayerController* CCSPlayerController::GetLocalPlayerController()
{

    const int nIndex = I::Engine->GetLocalPlayer();
    return I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(nIndex);
}

const Vector_t& CCSPlayerController::GetPawnOrigin()
{

    CBaseHandle hPawn = this->GetPawnHandle();
    if (!hPawn.IsValid())
        return vecEmpty;

    C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(hPawn);
    if (pPawn == nullptr)
        return vecEmpty;

    return pPawn->GetSceneOrigin();
}

C_BaseEntity* C_BaseEntity::GetLocalPlayer()
{

    const int nIndex = I::Engine->GetLocalPlayer();
    return I::GameResourceService->pGameEntitySystem->Get(nIndex);
}

const Vector_t& C_BaseEntity::GetSceneOrigin()
{
    if (this->GetGameSceneNode())
        return GetGameSceneNode()->GetAbsOrigin();

    return vecEmpty;
}
bool C_CSPlayerPawn::CalculateBoundingBox(ImVec4& out, bool compute_surrounding_box) {

    if (!this || this->GetHealth() <= 0)
        return false;

    CCollisionProperty* pCollision = this->GetCollision();
    if (pCollision == nullptr) {
        return false;
    }

    Vector_t min{}, max{};

    CTransform nodeToWorldTransform = this->GetGameSceneNode()->GetNodeToWorld();
    const Matrix3x4_t matTransform = nodeToWorldTransform.quatOrientation.ToMatrix(nodeToWorldTransform.vecPosition);
    if (compute_surrounding_box) {
        if (!ComputeHitboxSurroundingBox(min, max)) {
            return false;
        }
    }
    else {
        min = pCollision->GetMins();
        max = pCollision->GetMaxs();
    }

    out.x = out.y = std::numeric_limits<float>::max();
    out.z = out.w = -std::numeric_limits<float>::max();

    for (int i = 0; i < 8; ++i)
    {
        const Vector_t vecPoint{
            i & 1 ? max.x : min.x,
            i & 2 ? max.y : min.y,
            i & 4 ? max.z : min.z
        };
        ImVec2 vecScreen;
        if (!D::WorldToScreen(vecPoint.Transform(matTransform), vecScreen))
            return false;

        out.x = MATH::Min(out.x, vecScreen.x);
        out.y = MATH::Min(out.y, vecScreen.y);
        out.z = MATH::Max(out.z, vecScreen.x);
        out.w = MATH::Max(out.w, vecScreen.y);
    }

    return true;
}

bool C_CSPlayerPawn::hasArmour(const int hitgroup) {
    if (!this->GetItemServices())
        return false;

    switch (hitgroup) {
    case HITGROUP_HEAD:
        return this->GetItemServices()->m_bHasHelmet();
    case HITGROUP_GENERIC:
    case HITGROUP_CHEST:
    case HITGROUP_STOMACH:
    case HITGROUP_LEFTARM:
    case HITGROUP_RIGHTARM:
        return true;
    default:
        return false;
    }
}
bool C_CSPlayerPawn::IsOtherEnemy(C_CSPlayerPawn* pOther)
{
    // check are other player is invalid or we're comparing against ourselves
    if (pOther == nullptr || this == pOther)
        return false;

    if (CONVAR::game_type->value.i32 == GAMETYPE_FREEFORALL && CONVAR::game_mode->value.i32 == GAMEMODE_FREEFORALL_SURVIVAL)
        // check is not teammate
        return (this->GetSurvivalTeam() != pOther->GetSurvivalTeam());

    // @todo: check is deathmatch
    if (CONVAR::mp_teammates_are_enemies->value.i1)
        return true;

    return this->GetAssociatedTeam() != pOther->GetAssociatedTeam();
}
/*
bool CEconItemDefinition::IsWeapon() {
    // Every gun supports at least 4 stickers.
    return GetStickersSupportedCount() >= 4;
}

bool CEconItemDefinition::IsKnife(bool excludeDefault) {
    static constexpr auto CSGO_Type_Knife =
        HASHCNSX::hash_32_fnv1a_const("#CSGO_Type_Knife");

    if (HASHCNSX::hash_32_fnv1a_const(m_pszItemTypeName) != CSGO_Type_Knife)
        return false;

    return excludeDefault ? m_nDefIndex >= 500 : true;
}

bool CEconItemDefinition::IsGlove(bool excludeDefault) {
    static constexpr auto Type_Hands = HASHCNSX::hash_32_fnv1a_const("#Type_Hands");

    if (HASHCNSX::hash_32_fnv1a_const(m_pszItemTypeName) != Type_Hands) return false;
    const bool defaultGlove = m_nDefIndex == 5028 || m_nDefIndex == 5029;

    return excludeDefault ? !defaultGlove : true;
}
*/
int C_CSPlayerPawn::GetAssociatedTeam()
{
    const int nTeam = this->GetTeam();

    // @todo: check is coaching, currently cs2 doesnt have sv_coaching_enabled, so just let it be for now...
    //if (CONVAR::sv_coaching_enabled->GetBool() && nTeam == TEAM_SPECTATOR)
    //	return this->GetCoachingTeam();

    return nTeam;
}
#include "interfaces/itrace.h"
//48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 48 8B 59 08 41 8B F8 8B 69 10 8B 72 10 0F 29 74 24 20 0F 28 F3 48 8D 8B C0 00 00 00 81 39 00 02 00 00 7F 6D E8 ?? ?? ?? ?? 48 63 C8 48 C1 E1 06 48 03 8B D0 00 00 00 33 C0 48 C7 01 01"
/*bool C_CSPlayerPawn::SetupBones(matrix3x4* out, int maxBones, int boneMask, float currentTime) noexcept
{
    if (localPlayer && this == localPlayer.get() && localPlayer->isAlive())
    {
        uint32_t* effects = this->getEffects();
        uint32_t* shouldskipframe = (uint32_t*)((uintptr_t)this + 0xA68);
        uint32_t backup_effects = *effects;
        uint32_t backup_shouldskipframe = *shouldskipframe;
        *shouldskipframe = 0;
        *effects |= 8;
        auto result = VirtualMethod::call<bool, 13>(this + 4, out, maxBones, boneMask, currentTime);
        *effects = backup_effects;
        *shouldskipframe = backup_shouldskipframe;
        return result;
    }
    else
    {
        *reinterpret_cast<int*>(this + 0xA28) = 0;
        *reinterpret_cast<int*>(this + 0xA30) = memory->globalVars->framecount;
        int* render = reinterpret_cast<int*>(this + 0x274);
        uint32_t* shouldskipframe = (uint32_t*)((uintptr_t)this + 0xA68);
        uint32_t* effects = this->getEffects();
        int backup = *render;
        uint32_t backup_effects = *effects;
        *shouldskipframe = 0;
        *render = 0;
        boneMask |= 0x200;
        *effects |= 8;
        auto result = VirtualMethod::call<bool, 13>(this + 4, out, maxBones, boneMask, currentTime);
        *render = backup;
        *effects = backup_effects;
        return result;
    }
    return VirtualMethod::call<bool, 13>(this + 4, out, maxBones, boneMask, currentTime);
}*/

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
template<class T, class U>
T fine(T in, U low, U high)
{
    if (in <= low)
        return low;

    if (in >= high)
        return high;

    return in;
}
// basic extrap made in 3 mins gonna improve a lot later on its rly Important & needed
Vector_t extrapolate(C_CSPlayerPawn* ent, Vector_t pos) {
    if (!ent)
        return pos;

    auto simtime = ent->m_flSimulationTime();
    auto old_simtime = simtime + 4;
    float simtime_delta = simtime - old_simtime;
    int choked_ticks = fine(TIME_TO_TICKS(simtime_delta), 1, 15);

    Vector_t lastOrig;

    if (lastOrig.Length() != pos.Length())
        lastOrig = pos;

    float delta_distance = (pos - lastOrig).Length();
    Vector_t velocity_per_tick = ent->m_vecVelocity() * I::GlobalVars->flIntervalPerTick;
    Vector_t new_origin = pos + (velocity_per_tick * choked_ticks);
    return new_origin;
}
void C_CSPlayerPawn::CalculateHitboxData(uint32_t idx, Vector_t& pos, Vector4D_t& rot, float& scale, bool predict) {
    if (!this || this->GetHealth() <= 0)
        return;

    auto game_scene_node = this->GetGameSceneNode();
    if (!game_scene_node)
        return;

    auto skeleton = game_scene_node->GetSkeletonInstance();
    if (!skeleton)
        return;

    auto model_state = &skeleton->GetModel();
    if (!model_state)
        return;

    CStrongHandle<CModel> model = model_state->m_hModel();
    if (!model.is_valid())
        return;

    auto model_skelet = &model->m_modelSkeleton();
    if (!model_skelet)
        return;

    skeleton->calc_world_space_bones(0, bone_flags::FLAG_HITBOX);

    auto data = model_state->GetHitboxData();
    if (!data)
        return;

    if (!(model->GetHitboxFlags(idx) & bone_flags::FLAG_HITBOX))
        return;

    auto parent_index = model->GetHitboxParent(idx);
    if (parent_index == -1)
        return;

    rot = data[idx].rot;
    scale = data[idx].scale;


    if (predict) {
        // Extrapolate position based on velocity and acceleration
        pos = data[idx].pos + this->m_vecVelocity() * I::GlobalVars->flIntervalPerTick;
    }
    else {
        pos = data[idx].pos;
    }

}
uint32_t C_CSPlayerPawn::GetHitGroup(int idx) {
    switch (idx) {
    case HEAD:
    case NECK:
        return HITGROUP_HEAD;
    case CHEST:
    case LEFT_CHEST:
    case RIGHT_CHEST:
    case PELVIS: 
        return HITGROUP_CHEST;
    case STOMACH:
        return HITGROUP_STOMACH;
    case L_LEG:
    case R_LEG:
    case L_FEET:
    case R_FEET:
        return HITGROUP_LEFTLEG;
    default:
        return false;
    }
}
C_CSWeaponBase* C_CSPlayerPawn::ActiveWeapon() {
    CPlayer_WeaponServices* WeaponServices = this->GetWeaponServices();
    if (!WeaponServices)
        return nullptr;

    auto ActiveWeapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(WeaponServices->m_hActiveWeapon());
    if (!ActiveWeapon)
        return nullptr;

    return ActiveWeapon;
}

C_CSWeaponBase* CCSPlayerController::GetPlayerWeapon(C_CSPlayerPawn* pPlayer)
{
    if (!pPlayer || !pPlayer->GetWeaponServices())
        return nullptr;

    CBaseHandle hActiveWeapon = pPlayer->GetWeaponServices()->m_hActiveWeapon();

    if (!hActiveWeapon.IsValid())
        return nullptr;

    C_CSWeaponBase* pWeapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(hActiveWeapon);

    return pWeapon;
}

bool C_CSPlayerPawn::CanNextAttack(float svtime) {
    return false;
}
bool C_CSPlayerPawn::CanShoot(float svtime) {
    CPlayer_WeaponServices* WeaponServices = this->GetWeaponServices();
    if (!WeaponServices)
        return false;

    auto ActiveWeapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(WeaponServices->m_hActiveWeapon());
    if (!ActiveWeapon)
        return false;

    auto data = ActiveWeapon->datawep();
    if (!data)
        return false;

    if (ActiveWeapon->clip1() <= 0)
        return false;

    if (data->m_WeaponType() == WEAPONTYPE_KNIFE || data->m_WeaponType() == WEAPONTYPE_FISTS)
        return true;
    /*
    auto next_attack  = (WeaponServices->m_flNextAttack());
    auto next_2 =  WeaponServices->m_flNextAttack() * I::GlobalVars->flIntervalPerTick;
    auto next_3 = static_cast<float>(WeaponServices->m_flNextAttack() * I::GlobalVars->flIntervalPerTick);
    */

    auto primary_tick = ActiveWeapon->m_nNextPrimaryAttackTick();

    if (primary_tick > svtime) {
        return false;
    }

    return true;
}
bool C_CSPlayerPawn::Visible(C_CSPlayerPawn* local, int type, bool v)
{
    if (!this || this->GetHealth() <= 0)
        return false;

    if (type == 0) {
        trace_filter_t filter = {};
        I::Trace->Init(filter, local, 0x1C3003, 4, 7);

        game_trace_t trace = {};
        ray_t ray = {};

        Vector_t start_eye = local->GetEyePosition();
        Vector_t end_eye = this->GetEyePosition();

        I::Trace->TraceShape(ray, &start_eye, &end_eye, filter, trace);

        return trace.HitEntity && trace.HitEntity->GetRefEHandle().GetEntryIndex() == this->GetRefEHandle().GetEntryIndex() || trace.Fraction > 0.97f;
    }
    else if (type == TRACE_TYPE::AIMBOT) {
        if (v) {
            trace_filter_t filter = {};
            I::Trace->Init(filter, local, 0x1C3003, 4, 7);

            game_trace_t trace = {};
            ray_t ray = {};

            Vector_t start_eye = local->GetEyePosition();
            Vector_t end_eye = this->GetEyePosition();

            I::Trace->TraceShape(ray, &start_eye, &end_eye, filter, trace);

            return trace.HitEntity && trace.HitEntity->GetRefEHandle().GetEntryIndex() == this->GetRefEHandle().GetEntryIndex() || trace.Fraction > 0.97f;
        }
        else
            return true;
    }

    return false;
}
float C_CSPlayerPawn::GetProperSpread() {
    /* acc penalty */
    auto weapon = this->ActiveWeapon();
    if (!weapon)
        return 0.f;

    auto vdata = weapon->datawep();
    if (!vdata)
        return 0.f;
    float flSpread = 0.f;
    if (weapon->m_weaponMode() == CSWeaponMode::Primary_Mode)
    {
        return vdata->m_flSpread().flValue[0];
    }
    else if (weapon->m_weaponMode() == CSWeaponMode::Secondary_Mode)
    {
        return vdata->m_flSpread().flValue[1];
    }
    return flSpread;
};
float C_CSPlayerPawn::GetProperAccuracy() {
    /* acc penalty */
    auto weapon = this->ActiveWeapon();
    if (!weapon)
        return 0.f;

    auto vdata = weapon->datawep();
    if (!vdata)
        return 0.f;

    float flInnacuracy = weapon->m_fAccuracyPenalty();

    if (this->m_vecVelocity().Length2D() != 0.0f) /* we are not standing */
    {
        float flInnacuracyMove = 0.0f;

        if (weapon->m_weaponMode() == CSWeaponMode::Primary_Mode)
            flInnacuracyMove = vdata->m_flInaccuracyMove().flValue[0];
        else if (weapon->m_weaponMode() == CSWeaponMode::Secondary_Mode)
            flInnacuracyMove = vdata->m_flInaccuracyMove().flValue[1];

        if (this->m_bIsWalking())
            flInnacuracyMove /= 3.f;

        else if (this->GetFlags() & FL_DUCKING)
            flInnacuracyMove /= 6.f;

        flInnacuracy += flInnacuracyMove;
    }

    if (!(this->GetFlags() & FL_ONGROUND))
    {
        if (weapon->m_weaponMode() == CSWeaponMode::Primary_Mode)
            flInnacuracy += vdata->m_flInaccuracyJump().flValue[0];
        else if (weapon->m_weaponMode() == CSWeaponMode::Secondary_Mode)
            flInnacuracy += vdata->m_flInaccuracyJump().flValue[1];
    }

    return flInnacuracy;
};
bool C_CSPlayerPawn::InsideCrosshair(C_CSPlayerPawn* target, QAngle_t ang, float range)
{
    if (!this || this->GetHealth() <= 0)
        return false;


    trace_filter_t filter = {};
    I::Trace->Init(filter, target, 0x1C3003, 4, 7);

    game_trace_t trace = {};
    ray_t ray = {};
    Vector_t vecForward = { };
    ang.ToDirections(&vecForward);
    vecForward *= range;
    Vector_t vecStart = target->GetEyePosition();
    Vector_t vecEnd = vecStart + vecForward;
    I::Trace->TraceShape(ray, &vecStart, &vecEnd, filter, trace);

    return trace.HitEntity && trace.HitEntity->GetRefEHandle().GetEntryIndex() == this->GetRefEHandle().GetEntryIndex();
}

bool C_CSPlayerPawn::TracePoint(C_CSPlayerPawn* target, Vector_t end)
{
    if (!this || this->GetHealth() <= 0)
        return false;

    trace_filter_t filter = {};
    I::Trace->Init(filter, this, 0x1C3003, 4, 7);

    game_trace_t trace = {};
    ray_t ray = {};

    Vector_t vecStart = this->GetEyePosition();
    I::Trace->TraceShape(ray, &vecStart, &end, filter, trace);

    if (trace.HitEntity && trace.HitEntity->GetRefEHandle().GetEntryIndex() == target->GetRefEHandle().GetEntryIndex())
    {
        return true;
    }

    return false;
}


// cHoca

bool C_BaseEntity::ComputeHitboxSurroundingBox(Vector_t& min, Vector_t& max) {
    using fnComputeHitboxSurroundingBox = bool(CS_FASTCALL*)(void*, Vector_t&, Vector_t&);
    static auto ComputeHitboxSurroundingBox = reinterpret_cast<fnComputeHitboxSurroundingBox>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E9 ? ? ? ? F6 43 5B FD")), 1, 0));
    if (!ComputeHitboxSurroundingBox) {
        L_PRINT(LOG_ERROR) << CS_XOR("[C_BaseEntity::ComputeHitboxSurroundingBox] C_HOOK::ComputeHitboxSurroundingBox > null");
        return false;
    }
    return ComputeHitboxSurroundingBox(this, min, max);

}
//?GetNumberOfBoundContexts@SchedulerBase@details@Concurrency@@IEBAKXZ_0 ; Concurrency::details::SchedulerBase::GetNumberOfBoundContexts(void)
// Em baixo no SUBROUTINE de baixo

#include "../core/spoofcall/invoker.h"
void CSkeletonInstance::get_bone_data(bone_data& data, int index)
{
    // cHoca

    using fnBoneData = void(CS_FASTCALL*)(void*, bone_data&, int index);
    static auto BoneData = reinterpret_cast<fnBoneData>(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? EB 19 48 8B CF")));

    return BoneData(this, data, index);
}
// #STR: "C:\\buildworker\\csgo_rel_win64\\build\\src\\game\\shared\, "Bone merge bones from parent were invalid: parent model '%, "CalcWorldSpaceBones"
void CS_FASTCALL CSkeletonInstance::calc_world_space_bones(uint32_t parent, uint32_t mask)
{	// cHoca

    using fnNewCalcWSsBones = void(CS_FASTCALL)(void*, uint32_t);
    static auto bone_new = reinterpret_cast<fnNewCalcWSsBones*>(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC D0")));
    return bone_new(this, mask);
}

uint32_t CModel::GetHitboxesNum() {

    using fnHitboxNum = uint32_t(CS_FASTCALL*)(void*);
    static auto HitboxNum = reinterpret_cast<fnHitboxNum>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 8B D8 48 C7 44 24 50 00 00 00 00")), 1, 0));
    CS_ASSERT(HitboxNum != nullptr);

    return HitboxNum(this);

}

CGCClientSharedObjectCache* CGCClient::FindSOCache(SOID_t ID,
    bool bCreateIfMissing) {

    using fnFindSOCache = CGCClientSharedObjectCache * (CS_FASTCALL*)(void*, SOID_t, bool);
    static auto FindSOCache = reinterpret_cast<fnFindSOCache>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 10 48 89 6C 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 83 EC 40 48")));
    CS_ASSERT(FindSOCache != nullptr);

    return FindSOCache(this, ID, bCreateIfMissing);

}



void CGameSceneNode::SetMeshGroupMask(uint64_t meshGroupMask) {
    if (!H::SetMeshGroupMask) {
        L_PRINT(LOG_ERROR) << "error getting meshgroupmask";
    }

    return H::SetMeshGroupMask(this, meshGroupMask);
}

void C_BaseModelEntity::SetModel(const char* name) {
    auto orig = H::hkSetModel.GetOriginal();
    return orig(this, name);
}


void CEconItem::SetDynamicAttributeValue(int index, void* value) {
    CEconItemSchema* pItemSchema =
        I::Client->GetEconItemSystem()->GetEconItemSchema();
    if (!pItemSchema) return;

    void* pAttributeDefinitionInterface =
        pItemSchema->GetAttributeDefinitionInterface(index);
    if (!pAttributeDefinitionInterface) return;

    if (!H::fnSetDynamicAttributeValueUint) return;
    H::fnSetDynamicAttributeValueUint(this, pAttributeDefinitionInterface,
        value);
}

void CEconItem::SetDynamicAttributeValueString(int index, const char* value) {
    // CS2FIXME: Function got inlined and cannot be sigscanned.
}

CEconItem* CEconItem::CreateInstance() {
    // ida: // #STR: "Update(CEconItem)", "CEconItem", "Create(CEconItem)", "BuildCacheSubscribed(CEconItem)", "Update(CEconEquipSlot)", "CEconEquipSlot", "Create(CEconEquipSlot)", "BuildCacheSubscribed(CEconEquipSlot)", "Update(CEconPersonaDataPublic)", "CEconPersonaDataPublic"
    // ida:  sub_E0F420(
    //	1,
    //	(unsigned int)CEconItem::CreateInstance,
    //	0,
    //	(unsigned int)"CEconItem",
    //	(__int64)"BuildCacheSubscribed(CEconItem)",
    //	(__int64)"Create(CEconItem)",
    //	(__int64)"Update(CEconItem)");

    using fnCreateSharedObjectSubclassEconItem = CEconItem * (__cdecl*)();
    static fnCreateSharedObjectSubclassEconItem oCreateSharedObjectSubclassEconItem = reinterpret_cast<fnCreateSharedObjectSubclassEconItem>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85")));

#ifdef CS_PARANOID
    CS_ASSERT(oCreateSharedObjectSubclassEconItem != nullptr);
#endif

    return oCreateSharedObjectSubclassEconItem();
}
uint32_t CModel::GetHitboxFlags(uint32_t index)
{

    using fnHitboxFlags = uint32_t(CS_FASTCALL*)(void*, uint32_t);
    static auto HitboxFlags = reinterpret_cast<fnHitboxFlags>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 78 16 3B 91")));
    CS_ASSERT(HitboxFlags != nullptr);

    return HitboxFlags(this, index);
}

const char* CModel::GetHitboxName(uint32_t index)
{
    using fnHitboxName = const char* (CS_FASTCALL*)(void*, uint32_t);
    static auto HitboxName = reinterpret_cast<fnHitboxName>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 78 25 3B 91")));
    CS_ASSERT(HitboxName != nullptr);

    return HitboxName(this, index);
}

uint32_t CModel::GetHitboxParent(uint32_t index)
{

    using fnHitboxParent = uint32_t(CS_FASTCALL*)(void*, uint32_t);
    static auto HitboxParent = reinterpret_cast<fnHitboxParent>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 78 17 3B 91 78")));
    CS_ASSERT(HitboxParent != nullptr);
    return HitboxParent(this, index);
}

CGCClientSharedObjectTypeCache* CGCClientSharedObjectCache::CreateBaseTypeCache(
    int nClassID) {
    using fnCGCClientSharedObjectTypeCache = CGCClientSharedObjectTypeCache * (CS_FASTCALL*)(void*, int);
    static fnCGCClientSharedObjectTypeCache createbasetypecache = reinterpret_cast<fnCGCClientSharedObjectTypeCache>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 33 C9 8B D1")), 1, 0));
    CS_ASSERT(createbasetypecache != nullptr);
    return createbasetypecache(this, nClassID);
}


void C_CSWeaponBase::AddStattrakEntity()
{
    using fnAddStattrakEntity = void(CS_FASTCALL*)(void*);
    static auto hkAddStattrakEntity = reinterpret_cast<fnAddStattrakEntity>(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 55 41 55 48 8D 6C 24 B8")));
    CS_ASSERT(hkAddStattrakEntity != nullptr);
    return hkAddStattrakEntity(this);
}

void C_CSWeaponBase::AddNametagEntity()
{
    using fnAddNametagEntity = void(CS_FASTCALL*)(void*);
    static auto hkAddNametagEntity = reinterpret_cast<fnAddNametagEntity>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 48 8B C8 E8 ? ? ? ?")), 1, 0));
    CS_ASSERT(hkAddNametagEntity != nullptr);
    return hkAddNametagEntity(this);
}

CEconItem* C_EconItemView::GetSOCData(CCSPlayerInventory* sdfsdf) {
    CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
    if (!pInventory) return nullptr;

    return pInventory->GetSOCDataForItem(m_iItemID());
}


bool CEconItemDefinition::IsWeapon() {
    // Every gun supports at least 4 stickers.
    return GetStickersSupportedCount() >= 4;
}

bool CEconItemDefinition::IsKnife(bool excludeDefault, const char* name) {
    auto CSGO_Type_Knife = FNV1A::Hash("#CSGO_Type_Knife");

    if (FNV1A::Hash(this->m_pszItemBaseName) != CSGO_Type_Knife)
        return false;

    return excludeDefault ? m_nDefIndex >= 500 : true;
}

bool CEconItemDefinition::IsGlove(bool excludeDefault, const char* name) {
    auto Type_Hands = FNV1A::Hash("#Type_Hands");

    bool valid = FNV1A::Hash(this->m_pszItemBaseName) == Type_Hands;
    bool defaultGlove = valid && m_nDefIndex == 5028 || m_nDefIndex == 5029;

    return excludeDefault ? !defaultGlove : valid;
}
