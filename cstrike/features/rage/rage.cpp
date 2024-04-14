#include "rage.h"
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
#include "../misc/movement.h"
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
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "../../core/hooks.h"
#define M_PI_F ((float)(M_PI)) 
#ifndef RAD2DEG
#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
#endif

#ifndef DEG2RAD
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))
#endif
static std::vector <std::uint32_t> e_hitboxes;
static bool m_stop = false;
static bool early_stopped = false;

#include <mutex>
#define MAX_STUDIO_BONES 1024


enum class HitscanMode : int {
    NORMAL = 0,
    LETHAL = 1,
    LETHAL2 = 3,
    PREFER = 4
};

struct HitscanBox_t {
    int         m_index;
    HitscanMode m_mode;

    __forceinline bool operator==(const HitscanBox_t& c) const {
        return m_index == c.m_index && m_mode == c.m_mode;
    }
};

class target
{
public:
    C_CSPlayerPawn* handle;
    C_BaseEntity* entHandle;
    target()
    {
        handle = nullptr;
        entHandle = nullptr;
    }

    target(C_CSPlayerPawn* handle, C_BaseEntity* entHandle)
    {
        this->handle = handle;
        this->entHandle = entHandle;
    }
};

class temp_point
{
public:
    Vector_t point;
    QAngle_t pointangle;
    QAngle_t angle;

    int hitbox;
    bool center;
    float safe;
    float damage;
    bool canHit;

    temp_point()
    {
        point = Vector_t(0, 0, 0);
        pointangle = QAngle_t(0, 0, 0);
        angle = QAngle_t(0, 0, 0);
        hitbox = -1;
        center = false;
        safe = 0.0f;
        damage = 0.f;
        canHit = false;
    }

    temp_point(const Vector_t& point, const QAngle_t& pointangle, const QAngle_t& angle, const int& hitbox, const bool& center, const float& damage, const bool& canHit) //-V818 //-V730
    {
        this->point = point;
        this->pointangle = pointangle;
        this->angle = angle;
        this->hitbox = hitbox;
        this->center = center;
        this->damage = damage;
        this->canHit = canHit;
    }

    void reset()
    {
        point = Vector_t(0, 0, 0);
        pointangle = QAngle_t(0, 0, 0);
        angle = QAngle_t(0, 0, 0);
        hitbox = -1;
        center = false;
        safe = 0.0f;
        damage = 0.f;
        canHit = false;
    }
};
class scan_point
{
public:
    Vector_t point;
    QAngle_t pointangle;
    QAngle_t angle;

    int hitbox;
    bool center;
    float safe;
    float damage;
    bool canHit;

    scan_point()
    {
        point = Vector_t(0, 0, 0);
        pointangle = QAngle_t(0, 0, 0);
        angle = QAngle_t(0, 0, 0);
        hitbox = -1;
        center = false;
        safe = 0.0f;
        damage = 0.f;
        canHit = false;
    }

    scan_point(const Vector_t& point, const QAngle_t& pointangle, const QAngle_t& angle, const int& hitbox, const bool& center, const float& damage, const bool& canHit) //-V818 //-V730
    {
        this->point = point;
        this->pointangle = pointangle;
        this->angle = angle;
        this->hitbox = hitbox;
        this->center = center;
        this->damage = damage;
        this->canHit = canHit;
    }

    void reset()
    {
        point = Vector_t(0, 0, 0);
        pointangle = QAngle_t(0, 0, 0);
        angle = QAngle_t(0, 0, 0);
        hitbox = -1;
        center = false;
        safe = 0.0f;
        damage = 0.f;
        canHit = false;
    }
};
class scan_data
{
public:
    scan_point point;
    temp_point temp_point;

    bool visible;
    int damage;
    int hitbox;

    scan_data()
    {
        reset();
    }

    void reset()
    {
        temp_point.reset();
        point.reset();
        visible = false;
        damage = -1;
        hitbox = -1;
    }

    bool valid()
    {
        return damage >= 1 && point.angle.IsValid();
    }
};

class scanned_target
{
public:
    C_CSPlayerPawn* record;
    C_BaseEntity* ent;
    scan_data data;

    float fov;
    float distance;
    int health;
    float damage;
    scanned_target()
    {
        reset();
    }

    scanned_target(const scanned_target& data) //-V688
    {
        this->record = data.record;
        this->data = data.data;
        this->fov = data.fov;
        this->distance = data.distance;
        this->health = data.health;
        this->damage = data.damage;
        this->ent = data.ent;

    }

    scanned_target& operator=(const scanned_target& data) //-V688
    {
        this->ent = data.ent;
        this->record = data.record;
        this->data = data.data;
        this->fov = data.fov;
        this->distance = data.distance;
        this->health = data.health;
        this->damage = data.damage;

        return *this;
    }

    scanned_target(C_BaseEntity* ent, float damage, float health, Vector_t eyepos, Vector_t end_pos, C_CSPlayerPawn* record, const scan_data& data) //-V688 //-V818
    {
        this->ent = ent;
        this->record = record;
        this->data = data;
        this->distance = eyepos.DistTo(end_pos);
        this->fov = 0;
        this->health = health;
        this->damage = damage;
    }

    void reset()
    {
        ent = nullptr;
        record = nullptr;
        data.reset();
        fov = 0.0f;
        distance = 0.0f;
        health = 0;
        damage = 0;
    }
};


Vector_t last_shoot_position;
std::vector <scanned_target> scanned_targets;
scanned_target final_target;
C_CSPlayerPawn* last_target;

std::vector <target> targets;
// TODO: not sure if this is correct
float calc_lerp() noexcept {

    const   float cl_interp = I::Cvar->Find(FNV1A::Hash("cl_interp"))->value.fl;
    auto lerp = cl_interp / 64.f;

    if (lerp <= cl_interp)
        lerp = cl_interp;

    return lerp;
}

float get_las_valid_sim_time() noexcept {
    auto sv_maxunlag = I::Cvar->Find(FNV1A::Hash("sv_maxunlag"))->value.fl;

    const auto nci = I::Engine->GetNetChannelInfo(0);
    if (!nci)
        return std::numeric_limits<float>::max();

    const auto latency = nci->get_latency(flow::FLOW_OUTGOING) + nci->get_latency(FLOW_INCOMING);
    const float correct = std::clamp(latency, 0.0f, sv_maxunlag);
    const float max_delta = std::min((sv_maxunlag - correct), 0.2f) / 1000.f;

    return I::GlobalVars->flCurtime - max_delta;
}

struct record {
public:
    bool valid = { };
    float simulation_time = { };
    std::array<bone_data, MAX_STUDIO_BONES> bone_data = { };
    Vector_t head_pos = { };
};

struct record_set {
public:
    record* record1;
    record* record2;
    float fraction;
};

class backtrack_entity {
public:
    CBaseHandle handle = {};
    std::vector<record> records = {};
    bool last_valid = { };
    CStrongHandle<CModel> model = { };
    uint32_t num_bones = { };
    CSkeletonInstance* skeleton;
    backtrack_entity(CBaseHandle handle) noexcept : handle(handle) {
        CS_ASSERT(I::GlobalVars);
        auto unlag = I::Cvar->Find(FNV1A::Hash(CS_XOR("sv_maxunlag")))->value.fl;
        auto max_ticks = static_cast<int>(unlag / I::GlobalVars->flIntervalPerTick);

        if (max_ticks < 0)
            max_ticks = 0;

        records.resize(max_ticks);
        records.resize(records.capacity()); // basically free, so why not
    }

    // TODO: not sure if this is correct
    float calc_lerp() noexcept {

        auto cl_interp = I::Cvar->Find(FNV1A::Hash("cl_interp"))->value.fl;
        auto lerp = cl_interp / 64.f;

        if (lerp <= cl_interp)
            lerp = cl_interp;

        return lerp;
    }

    float get_las_valid_sim_time() noexcept {
        auto sv_maxunlag = I::Cvar->Find(FNV1A::Hash("sv_maxunlag"))->value.fl;

        const auto nci = I::Engine->GetNetChannelInfo(0);
        if (!nci)
            return std::numeric_limits<float>::max();

        const auto latency = nci->get_latency(flow::FLOW_OUTGOING) + nci->get_latency(FLOW_INCOMING);
        const float correct = std::clamp(latency , 0.0f, sv_maxunlag);
        const float max_delta = std::min((sv_maxunlag - correct), 0.2f) / 1000.f;

        return I::GlobalVars->flCurtime - max_delta;
    }

    void save_data(unsigned int index) noexcept {
        if (records.size() <= 0)
            return;

        auto& rec = records[index % records.size()];

        if (!SDK::LocalPawn)
            return;

        C_CSPlayerPawn* player = final_target.record;
        if (!player || player->GetHealth() <= 0 || player->GetGameSceneNode()->IsDormant()) {
            invalidate();
            return;
        }

        if (!SDK::LocalPawn->IsOtherEnemy(player)) {
            invalidate();
            return;
        }

        rec.simulation_time = player->m_flSimulationTime();
        rec.head_pos = player->GetEyePosition();
       
        const auto game_scene_node = player->GetGameSceneNode();
        if (!game_scene_node) {
            invalidate();
            return;
        }

        skeleton = game_scene_node->GetSkeletonInstance();
        if (!skeleton) {
            invalidate();
            return;
        }

        skeleton->calc_world_space_bones(0, bone_flags::FLAG_HITBOX);

        auto& model_state = skeleton->GetModel();
        model = model_state.m_hModel();
        if (!model.is_valid()) {
            invalidate();
            return;
        }

        num_bones = model->GetHitboxesNum();
        if (num_bones == 0) {
            invalidate();
            return;
        }

        auto bone_data = model_state.GetHitboxData();

        if (num_bones > MAX_STUDIO_BONES)
            num_bones = MAX_STUDIO_BONES;

        memcpy(rec.bone_data.data(), bone_data, sizeof(bone_data) * num_bones);
        rec.valid = true;
        last_valid = true;
    }

    float closest_record(Vector_t start, Vector_t end, float las_valid_sim_time) {
        float closest = std::numeric_limits<float>::max();

        for (auto& rec : records) {
            if (!rec.valid)
                continue;

            if (rec.simulation_time <= las_valid_sim_time)
                continue;

            float dist = get_distance(rec.head_pos, start, end);
            if (dist < closest)
                closest = dist;
        }

        return closest;
    }

    record_set find_best_records(Vector_t start, Vector_t end, float las_valid_sim_time) {

        int best_bone = -1;
        int best_record = -1;
        float best_bone_dist = std::numeric_limits<float>::max();

        for (int bone_index = 0; bone_index < (int)num_bones; bone_index++) {
            if (!(model->GetHitboxFlags(bone_index) & bone_flags::FLAG_HITBOX)) {
                continue;
            }

            auto parent_bone_index = model->GetHitboxParent(bone_index);
            if (parent_bone_index == -1)
                continue;

            for (int record_index = 0; record_index < std::size(records); record_index++) {
                const auto& rec = records[record_index];
                if (!rec.valid)
                    continue;

                if (rec.simulation_time <= las_valid_sim_time)
                    continue;

                const float bone_dist = MATH::segment_dist(start, end, rec.bone_data[bone_index].pos, rec.bone_data[parent_bone_index].pos);
                if (bone_dist < best_bone_dist) {
                    best_bone_dist = bone_dist;
                    best_bone = bone_index;
                    best_record = record_index;
                }
            }
        }

        CS_ASSERT(best_bone != -1 && best_record != -1);

        int prev_record = best_record - 1;
        if (prev_record < 0)
            prev_record += records.size();
        if (!records[prev_record].valid)
            prev_record = best_record;

        int next_record = (best_record + 1) % records.size();
        if (!records[next_record].valid)
            next_record = best_record;

        const auto& best_rec = records[best_record];
        uint32_t best_bone_parent = model->GetHitboxParent(best_bone);


        float prev_dist = MATH::segment_dist(
            start, end,
            records[prev_record].bone_data[best_bone].pos,
            records[prev_record].bone_data[best_bone_parent].pos
        );
        float next_dist = MATH::segment_dist(
            start, end,
            records[next_record].bone_data[best_bone].pos,
            records[next_record].bone_data[best_bone_parent].pos
        );

        bool prev_is_best = prev_dist < next_dist;
        int rec1_index = prev_is_best ? prev_record : best_record;
        int rec2_index = prev_is_best ? best_record : next_record;

        // records are the same
        if (rec1_index == rec2_index)
            return { &records[rec1_index], &records[rec2_index], 0.0f };

        const float best_full_dist = (prev_is_best ? prev_dist : next_dist) + best_bone_dist;
        const float fraction = prev_is_best ? prev_dist / best_full_dist : best_bone_dist / best_full_dist;

        return { &records[rec1_index], &records[rec2_index], fraction };
    }

private:

    void invalidate() noexcept {
        if (last_valid) {
            for (auto& rec : records)
                rec.valid = false;

            last_valid = false;
        }
    }

    float get_distance(Vector_t p, Vector_t a, Vector_t b) {
        Vector_t ap = p - a;
        Vector_t ab = b - a;

        float ab2 = ab.DotProduct(ab);
        float ap_ab = ap.DotProduct(ab);
        float t = ap_ab / ab2;

        // ignore if player is behind ur or too far away
        if (t < 0.0f || t > 1.0f)
            return std::numeric_limits<float>::max();

        Vector_t nearest = a + ab * t;
        return (p - nearest).Length();
    }
};

std::unordered_map<int, std::unique_ptr<backtrack_entity>> records{ };
void F::LAGCOMP::impl::Initialize() {
    EntCache::RegisterCallback(
        CachedEntity_t::Type::PLAYER_CONTROLLER,
        [](int index, const CBaseHandle handle) noexcept {
            records.insert_or_assign(index, std::make_unique<backtrack_entity>(handle));
        },
        [](int index, const CBaseHandle handle) noexcept {
            records.erase(index);
        }
        );
}
void F::LAGCOMP::impl::Render() noexcept {

    const float las_valid_sim_time = get_las_valid_sim_time();
    for (const auto& [index, entity] : records) {
        if (!entity->last_valid)
            continue;

        record const* last_record = nullptr;
        float last_record_time = std::numeric_limits<float>::max();

        for (const auto& rec : entity->records) {
            if (!rec.valid || rec.simulation_time <= las_valid_sim_time)
                continue;

            if (rec.simulation_time < last_record_time) {
                last_record_time = rec.simulation_time;
                last_record = &rec;
            }
        }

        if (!last_record)
            continue;

        L_PRINT(LOG_INFO) << CS_XOR("[record] info | simulation_time: ") << last_record->simulation_time;
        L_PRINT(LOG_INFO) << CS_XOR("[record] info | bone_data: ") << last_record->bone_data[HEAD].pos;
        L_PRINT(LOG_INFO) << CS_XOR("[record] info | head_pos: ") << last_record->head_pos;

        entity->skeleton->calc_world_space_bones(0, bone_flags::FLAG_HITBOX);

        for (uint32_t i = 0; i < entity->num_bones; i++) {
            if (!(entity->model->GetHitboxFlags(i) & bone_flags::FLAG_HITBOX)) {
                continue;
            }

            auto parent_index = entity->model->GetHitboxParent(i);
            if (parent_index == -1)
                continue;

            ImVec2 screen_pos, screen_parent_pos;
            if (!MATH::WorldToScreen(last_record->bone_data[i].pos, screen_pos))
                continue;
            if (!MATH::WorldToScreen(last_record->bone_data[parent_index].pos, screen_parent_pos))
                continue;

            C_GET(ColorPickerVar_t, Vars.colSkeletonOutline).colValue.a = C_GET(ColorPickerVar_t, Vars.colSkeleton).colValue.a;
            D::pDrawListActive->AddLine(ImVec2(screen_pos.x, screen_pos.y), ImVec2(screen_parent_pos.x, screen_parent_pos.y), C_GET(ColorPickerVar_t, Vars.colSkeletonOutline).colValue.GetU32(), 2.f);
            D::pDrawListActive->AddLine(ImVec2(screen_pos.x, screen_pos.y), ImVec2(screen_parent_pos.x, screen_parent_pos.y), C_GET(ColorPickerVar_t, Vars.colSkeleton).colValue.GetU32(), 1.f);

        }
    }
}
void F::LAGCOMP::impl::Start(CUserCmd* cmd) noexcept {

    if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
        return;

    if (!SDK::LocalController || !SDK::LocalPawn)
        return;

    auto pLocal = SDK::LocalPawn;
    if (!pLocal || pLocal->GetHealth() <= 0)
        return;

    for (auto& [index, entity] : records) {
        entity->save_data(cmd->m_csgoUserCmd.m_pBaseCmd->m_nCommandNumber);
    }

    CPlayer_WeaponServices* WeaponServices = pLocal->GetWeaponServices();
    if (!WeaponServices)
        return;

    auto active_weapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(WeaponServices->m_hActiveWeapon());
    if (!active_weapon)
        return;

    auto v_data = active_weapon->datawep();
    if (!v_data || v_data->m_WeaponType() == WEAPONTYPE_MELEE)
        return;

    if (!cmd)
        return;

    if (cmd->m_csgoUserCmd.m_nAttack1StartHhistoryIndex == -1)
        return;

    const auto sub_tick = cmd->GetInputHistoryEntry(cmd->m_csgoUserCmd.m_nAttack1StartHhistoryIndex);

    if (!sub_tick || !sub_tick->m_sv_interp0 || !sub_tick->m_sv_interp1) {
        L_PRINT(LOG_ERROR) << (CS_XOR("invalid sub tick"));
        return;
    }

    Vector_t forward = MATH::angle_vector(sub_tick->m_pViewCmd->m_angValue); //sub_tick[1]-->->angValue.x, sub_tick->pViewCmd->angValue.y, sub_tick->pViewCmd->angValue.z);
    Vector_t start_pos = pLocal->GetEyePosition();
    Vector_t end_pos = start_pos + (forward * v_data->m_flRange());
    const float las_valid_sim_time = get_las_valid_sim_time();

    L_PRINT(LOG_INFO) << CS_XOR("[lagcomp] get_las_valid_sim_time: ") << get_las_valid_sim_time();

    int target_index = -1;
    float target_index_dist = std::numeric_limits<float>::max();
    for (auto& [index, entity] : records) {
        if (!entity->last_valid)
            continue;

        const float distance = entity->closest_record(start_pos, end_pos, las_valid_sim_time);
        if (distance < target_index_dist) {
            target_index = index;
            target_index_dist = distance;
            L_PRINT(LOG_WARNING) << CS_XOR("[lagcomp] Inserted new best record | idx: ") << target_index << " at distanc:" << target_index_dist;

        }
    }

    // no entities or to far away
    if (target_index == -1 || target_index_dist > 100.0f)
        return;


    const auto entity = records[target_index].get();

    const auto records = entity->find_best_records(start_pos, end_pos, las_valid_sim_time);

    const float time_per_tick = I::GlobalVars->flIntervalPerTick;
    const float best_time = std::lerp(records.record1->simulation_time, records.record2->simulation_time, records.fraction);
    const float best_tick_time = best_time / time_per_tick;
    int best_tick = (int)std::floor(best_tick_time);
    const float best_tick_fraction = best_tick_time - best_tick;

    best_tick -= 1; // FIXME: engine prediction?
    L_PRINT(LOG_INFO) << CS_XOR("[lagcomp] backtracking info | best tick: ") << best_tick << CS_XOR(" | tick fraction:") << best_tick_fraction << CS_XOR(" ticks:") << (I::GlobalVars->flCurtime - best_time) * 1000.f;



    if (!sub_tick->m_cl_interp || !sub_tick->m_sv_interp0 || !sub_tick->m_sv_interp1) {
        L_PRINT(LOG_INFO) << "Invalid sub tick";
        return;
    }

    L_PRINT(LOG_INFO) << CS_XOR("[subtick] subtick info: ");

    L_PRINT(LOG_INFO) << CS_XOR("[subtick] cl_interp info | cl_interp tick: ") << sub_tick->m_cl_interp->m_nDstTick;
    L_PRINT(LOG_INFO) << CS_XOR("[subtick] cl_interp info | nSrcTick tick: ") << sub_tick->m_cl_interp->m_nSrcTick;
    L_PRINT(LOG_INFO) << CS_XOR("[subtick] cl_interp info | flFraction tick: ") << sub_tick->m_cl_interp->m_flFraction;

    L_PRINT(LOG_INFO) << CS_XOR("[subtick] sv_interp0 info | cl_interp tick: ") << sub_tick->m_sv_interp0->m_nDstTick;
    L_PRINT(LOG_INFO) << CS_XOR("[subtick] sv_interp0 info | nSrcTick tick: ") << sub_tick->m_sv_interp0->m_nSrcTick;
    L_PRINT(LOG_INFO) << CS_XOR("[subtick] sv_interp0 info | flFraction tick: ") << sub_tick->m_sv_interp0->m_flFraction;

    L_PRINT(LOG_INFO) << CS_XOR("[subtick] sv_interp1 info | cl_interp tick: ") << sub_tick->m_sv_interp1->m_nDstTick;
    L_PRINT(LOG_INFO) << CS_XOR("[subtick] sv_interp1 info | nSrcTick tick: ") << sub_tick->m_sv_interp1->m_nSrcTick;
    L_PRINT(LOG_INFO) << CS_XOR("[subtick] sv_interp1 info | flFraction tick: ") << sub_tick->m_sv_interp1->m_flFraction;

    sub_tick->m_cl_interp->m_nSrcTick = best_tick;
    sub_tick->m_cl_interp->m_nDstTick = best_tick + 1;
    sub_tick->m_cl_interp->m_flFraction = best_tick_fraction;

    sub_tick->m_sv_interp0->m_nSrcTick = best_tick;
    sub_tick->m_sv_interp0->m_nDstTick = best_tick + 1;
    sub_tick->m_sv_interp0->m_flFraction = 0.f;

    sub_tick->m_sv_interp1->m_nSrcTick = best_tick + 1;
    sub_tick->m_sv_interp1->m_nDstTick = best_tick + 2;
    sub_tick->m_sv_interp1->m_flFraction = 0.f;

       
}


// setup config hitboxes
void F::RAGE::impl::Scan() {

    /* emplace menu hitboxes which will be used for hitscan*/

    if (rage_data.hitbox_head) {
        e_hitboxes.emplace_back(HEAD);
    }

    if (rage_data.hitbox_chest) {
        e_hitboxes.emplace_back(CHEST);
        e_hitboxes.emplace_back(RIGHT_CHEST);
        e_hitboxes.emplace_back(LEFT_CHEST);
    }

    if (rage_data.hitbox_stomach) {
        e_hitboxes.emplace_back(STOMACH);
        e_hitboxes.emplace_back(CENTER);
        e_hitboxes.emplace_back(PELVIS);
    }

    if (rage_data.hitbox_legs) {
        e_hitboxes.emplace_back(L_LEG);
        e_hitboxes.emplace_back(R_LEG);
    }
    if (rage_data.hitbox_feets) {
        e_hitboxes.emplace_back(L_FEET);
        e_hitboxes.emplace_back(R_FEET);
    }

    return;
}
void F::RAGE::impl::Reset(reset type) {
    switch (type) {
    case reset::entity:
        targets.clear();
        break;
    case reset::aimbot:
        e_hitboxes.clear();
        targets.clear();
        scanned_targets.clear();
        break;
    }
    return;
}

void F::RAGE::impl::SetupTarget(C_CSPlayerPawn* pLocal)
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
    const std::lock_guard<std::mutex> guard{ g_cachedEntitiesMutex };

    for (const auto& it : g_cachedEntities) {

        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(it.m_handle);
        if (pEntity == nullptr)
            continue;

        CBaseHandle hEntity = pEntity->GetRefEHandle();
        if (hEntity != it.m_handle) continue;

        if (it.m_type != CachedEntity_t::PLAYER_CONTROLLER)
            continue;

        CCSPlayerController* CPlayer = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(hEntity);
        if (CPlayer == nullptr)
            break;

        C_CSPlayerPawn* player = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(CPlayer->GetPawnHandle());
        if (!player)
            continue;

        if (player->GetHealth() <= 0 || !pLocal->IsOtherEnemy(player) || player->GetGameSceneNode()->IsDormant() || player->m_bGunGameImmunity())
            continue;

        targets.push_back(target(player, pEntity));
        continue;
    }



}

bool F::RAGE::impl::valid(C_CSPlayerPawn* player, C_CSPlayerPawn* pLocal, bool check) {
    if (!player)
        return false;

    bool Invalid = player->GetHealth() <= 0 || player->GetLifeState() == ELifeState::LIFE_DISCARDBODY || player->GetLifeState() == ELifeState::LIFE_DEAD || player->GetLifeState() == ELifeState::LIFE_DYING;
    if (Invalid || !pLocal->IsOtherEnemy(player) || player->GetGameSceneNode()->IsDormant() || player->m_bGunGameImmunity())
        return false;

    return true;
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


std::vector <scan_point> F::RAGE::impl::get_points(C_CSPlayerPawn* pLocal, C_CSPlayerPawn* record, uint32_t hitbox, bool from_aim)// надо добавить хтбоксы тут не все
{
    // removed
    std::vector <scan_point> points;
    return points;
}

void F::RAGE::impl::ScanPoint(C_CSPlayerPawn* pLocal, CUserCmd* cmd, C_CSPlayerPawn* record, scan_data& data, const Vector_t& shoot_pos, bool optimized) {
    auto weapon = pLocal->ActiveWeapon();
    if (!weapon)
        return;

    auto weapon_info = weapon->datawep();

    if (!weapon_info)
        return;

    if (e_hitboxes.empty())
        return;

    early_stopped = false;

    auto best_damage = 0;

    auto minimum_damage = rage_data.minimum_damage;

    float damage = 0.f;
    bool can_hit = false;

    std::vector <scan_point> points;
    std::vector <temp_point> temp_points;

    for (auto& hitbox : e_hitboxes)
    {
        float hitbox_scale = {}; QAngle_t temp_angle = {};  Vector_t hitbox_pos = {};  Vector4D_t hitbox_rot = {}; QAngle_t point_angle = {}; QAngle_t angle = {};

        // create our ange based on our current point
        record->CalculateHitboxData(hitbox, hitbox_pos, hitbox_rot, hitbox_scale, true);

        // shit got in vec convert to a quarention angle
        auto vec = get_target_angle(pLocal, hitbox_pos);
        angle.ToVec(vec);

        damage = rage->ScaleDamage(record, pLocal, weapon, hitbox_pos, damage, can_hit);


        if (damage <= 0) {
            temp_points.emplace_back(temp_point(hitbox_pos, point_angle, angle, hitbox, false, damage, can_hit));
            continue;
        }
        else {
            MATH::VectorAngless(pLocal->GetEyePosition() - hitbox_pos, point_angle);
            points.emplace_back(scan_point(hitbox_pos, point_angle, angle, hitbox, false, damage, can_hit));
        }
    }

    for (auto& temp_point : temp_points)
    {
        if (temp_point.damage > 0)
            continue;

        data.temp_point = temp_point;
    }

    if (points.empty())
        return;

    auto current_minimum_damage = minimum_damage;

    for (auto& point : points)
    {
        if (point.damage <= 0)
            continue;

        if (point.damage >= current_minimum_damage && point.damage >= best_damage) {
            best_damage = point.damage;
            data.point = point;
            data.damage = point.damage;
            data.hitbox = point.hitbox;

        }
    }
}


void F::RAGE::impl::ScanTarget(C_CSPlayerPawn* pLocal, CUserCmd* cmd, QAngle_t viewangles)
{
    if (!pLocal)
        return;

    auto weapon = pLocal->ActiveWeapon();
    if (!weapon)
        return;

    if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
        return;

    if (targets.size() <= 0)
        return;

    final_target.reset();

    for (auto& target : targets)
    {
        auto player = target.handle;

        if (!player)
            continue;

        if (!valid(player, pLocal))
            continue;

        scan_data last_data;

        rage->ScanPoint(pLocal, cmd, player, last_data, Vector_t(0, 0, 0), true);
      
        if (last_data.damage > 0) {
            m_stop = cheat->onground && cheat->canShot;
            rage->AutomaticStop(pLocal, weapon, cmd, early);
        }
        scanned_targets.emplace_back(scanned_target(target.entHandle, last_data.damage, player->GetHealth(), pLocal->GetEyePosition(), player->GetEyePosition(), player, last_data));
    }
}

bool F::RAGE::impl::Ready(C_CSPlayerPawn* pLocal) {
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

void F::RAGE::impl::Events(IGameEvent* ev, events type) {
    if (!C_GET(bool, Vars.rage_enable))
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
            // reset targets
            scanned_targets.clear();
            targets.clear();
        }
    }
                     break;
    case round_start: {
        rage->Reset(reset::entity);
    }
                    break;
    }
}



inline Vector_t CrossProduct(const Vector_t& a, const Vector_t& b)
{
    return Vector_t(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

#define SMALL_NUM 0.00000001 // anything that avoids division overflow
#define dot(u, v) ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
#define norm(v) sqrt(dot(v, v)) // norm = length of  vector
void VectorAngles(const Vector_t& forward, Vector_t& up, Vector_t& angles)
{
    Vector_t left = CrossProduct(up, forward);
    left.NormalizeInPlace();

    float forwardDist = forward.Length2D();

    if (forwardDist > 0.001f)
    {
        angles.x = atan2f(-forward.z, forwardDist) * 180 / MATH::_PI;
        angles.y = atan2f(forward.y, forward.x) * 180 / MATH::_PI;

        float upZ = (left.y * forward.x) - (left.x * forward.y);
        angles.z = atan2f(left.z, upZ) * 180 / MATH::_PI;
    }
    else
    {
        angles.x = atan2f(-forward.z, forwardDist) * 180 / MATH::_PI;
        angles.y = atan2f(-left.x, left.y) * 180 / MATH::_PI;
        angles.z = 0;
    }
}

void sin_cos(float radian, float* sin, float* cos)
{
    *sin = std::sin(radian);
    *cos = std::cos(radian);
}

float GRD_TO_BOG(float GRD)
{
    return (MATH::_PI / 180) * GRD;
}
void AngleVectors(const Vector_t& angles, Vector_t* forward)
{
    // sry
    //Assert(s_bMathlibInitialized);
    //Assert(forward);

    float sp, sy, cp, cy;

    sy = sin(DEG2RAD(angles[1]));
    cy = cos(DEG2RAD(angles[1]));

    sp = sin(DEG2RAD(angles[0]));
    cp = cos(DEG2RAD(angles[0]));

    forward->x = cp * cy;
    forward->y = cp * sy;
    forward->z = -sp;
}

void AngleVectors(const Vector_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up)
{
    float sp, sy, sr, cp, cy, cr;

    sin_cos(GRD_TO_BOG(angles.x), &sp, &cp);
    sin_cos(GRD_TO_BOG(angles.y), &sy, &cy);
    sin_cos(GRD_TO_BOG(angles.z), &sr, &cr);

    if (forward != nullptr)
    {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }

    if (right != nullptr)
    {
        right->x = -1 * sr * sp * cy + -1 * cr * -sy;
        right->y = -1 * sr * sp * sy + -1 * cr * cy;
        right->z = -1 * sr * cp;
    }

    if (up != nullptr)
    {
        up->x = cr * sp * cy + -sr * -sy;
        up->y = cr * sp * sy + -sr * cy;
        up->z = cr * cp;
    }
}



float F::RAGE::impl::GetSpread(C_CSWeaponBase* weapon)
{
    return weapon->get_spread();
}

void F::RAGE::impl::BuildSeed()
{
    for (auto i = 0; i <= 255; i++) {
        MATH::fnRandomSeed(i + 1);

        const auto pi_seed = MATH::fnRandomFloat(0.f, 6.283186f);

        F::RAGE::rage->m_computed_seeds.emplace_back(MATH::fnRandomFloat(0.f, 1.f),
            pi_seed);
    }
}

float F::RAGE::impl::GetInaccuracy(C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon)
{
    return weapon->get_inaccuracy();
}

float F::RAGE::impl::HitchanceFast(C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon)
{
    if (!weapon)
        return 0.f;

    if (!weapon->datawep())
        return 0.f;

    float flInaccuracy = this->GetInaccuracy(pLocal, weapon);
    flInaccuracy = 1 / flInaccuracy;

    return flInaccuracy;
}
void NormalizeAngles(Vector_t& angles)
{
    for (auto i = 0; i < 3; i++)
    {
        while (angles[i] < -180.0f)
            angles[i] += 360.0f;
        while (angles[i] > 180.0f)
            angles[i] -= 360.0f;
    }
}


__forceinline Vector_t CalculateSpread(C_CSWeaponBase* weapon, int seed, float inaccuracy, float spread, bool revolver2 = false) {
    const char* item_def_index;
    float      recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;

    if (!weapon)
        return { };
    // if we have no bullets, we have no spread.
    auto wep_info = weapon->datawep();
    if (!wep_info)
        return { };

    // get some data for later.
    item_def_index = wep_info->m_szName();
    recoil_index = weapon->m_flRecoilIndex();

    MATH::fnRandomSeed((seed & 0xff) + 1);

    // generate needed floats.
    r1 = MATH::fnRandomFloat(0.f, 1.f);
    r2 = MATH::fnRandomFloat(0.f, 3.14159265358979323846264338327950288f * 2);
    r3 = MATH::fnRandomFloat(0.f, 1.f);
    r4 = MATH::fnRandomFloat(0.f, 3.14159265358979323846264338327950288f * 2);

    // revolver secondary spread.
    if (item_def_index == CS_XOR("weapon_revoler") && revolver2) {
        r1 = 1.f - (r1 * r1);
        r3 = 1.f - (r3 * r3);
    }

    // negev spread.
    else if (item_def_index == CS_XOR("weapon_negev") && recoil_index < 3.f) {
        for (int i{ 3 }; i > recoil_index; --i) {
            r1 *= r1;
            r3 *= r3;
        }

        r1 = 1.f - r1;
        r3 = 1.f - r3;
    }

    // get needed sine / cosine values.
    c1 = std::cos(r2);
    c2 = std::cos(r4);
    s1 = std::sin(r2);
    s2 = std::sin(r4);

    // calculate spread vector.
    return {
        (c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
        (s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
        0.f
    };
}

bool F::RAGE::impl::CanHit(Vector_t start, Vector_t end, C_CSPlayerPawn* pLocal, C_CSPlayerPawn* record, int box)
{
    if (!pLocal)
        return false;

    if (!record)
        return false;


    return false;
}
void F::RAGE::impl::Hitchance(C_CSPlayerPawn* pLocal, C_CSPlayerPawn* ent, C_CSWeaponBase* weapon, QAngle_t vAimpoint)
{
    if (!pLocal)
        return;


    if (!weapon)
        return;

    auto data = weapon->datawep();
    if (!data)
        return;

    float HITCHANCE_MAX = 100.f;
    constexpr int   SEED_MAX = 255;

    Vector_t     start{ pLocal->GetEyePosition() }, end, fwd, right, up, dir, wep_spread;
    float      inaccuracy, spread;
    float hitchance = rage_data.rage_hitchance;
    Vector_t point = Vector_t(vAimpoint.x, vAimpoint.y, vAimpoint.z);

    if (hitchance <= 0) {
        rage_data.accurate = true;
        return;
    }

    // we cant reach it anymore too far 
    if (pLocal->GetEyePosition().DistTo(point) > data->m_flRange()) {
        rage_data.accurate = false;
        return;
    }

    size_t     total_hits{ }, needed_hits{ (size_t)std::ceil((hitchance * SEED_MAX) / HITCHANCE_MAX) };

    // get needed directional vectors.
    MATH::anglevectors(vAimpoint, &fwd, &right, &up);

    // store off inaccuracy / spread ( these functions are quite intensive and we only need them once ).
    inaccuracy = this->GetInaccuracy(pLocal, weapon);
    spread = this->GetSpread(weapon);

    // iterate all possible seeds.
    for (int i{ }; i <= SEED_MAX; ++i) {
        // get spread.
        wep_spread = CalculateSpread( weapon, i, inaccuracy, spread);

        // get spread direction.
        dir = (fwd + (right * wep_spread.x) + (up * wep_spread.y)).normalized();

        // get end of trace.
        end = start - (dir * data->m_flRange());

        trace_filter_t filter = {};
        I::Trace->Init(filter, pLocal, MASK_SHOT, 3, 7);

        game_trace_t trace = {};
        ray_t ray = {};

        I::Trace->TraceShape(ray, &start, &end, filter, trace);
        I::Trace->ClipTraceToPlayers(start, end, &filter, &trace, 0.F, 60.F, (1.F / (start - end).Length()) * (trace.m_end_pos - start).Length());

        if (trace.HitEntity && trace.HitEntity->GetRefEHandle().GetEntryIndex() == ent->GetRefEHandle().GetEntryIndex())
            ++total_hits;

        // we made it.
        if (total_hits >= needed_hits) {
            rage_data.accurate = true;

        }
        // we cant make it anymore.
        if ((SEED_MAX - i + total_hits) < needed_hits) {
            rage_data.accurate = false;

        }
    }
}

void VectorAngles2(const Vector_t& forward, Vector_t& angles)
{
    float tmp, yaw, pitch;

    if (forward[1] == 0 && forward[0] == 0)
    {
        yaw = 0;
        if (forward[2] > 0)
            pitch = 270;
        else
            pitch = 90;
    }
    else
    {
        yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
        if (yaw < 0)
            yaw += 360;

        tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
        pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
        if (pitch < 0)
            pitch += 360;
    }

    angles[0] = pitch;
    angles[1] = yaw;
    angles[2] = 0;
}
// Linear interpolation function
template <typename T>
T Lerp(float t, const T& start, const T& end) {
    return start + t * (end - start);
}

const char* GetExtractedWeaponName(C_CSWeaponBase* weapon)
{
    if (!weapon)
        return "";

    auto weapon_data = weapon->datawep();
    if (!weapon_data)
        return "";

    const char* szWeaponName = weapon_data->m_szName();
    const char* weaponPrefix = ("weapon_");
    const char* weaponNameStart = strstr(szWeaponName, weaponPrefix);
    const char* extractedWeaponName = weaponNameStart ? weaponNameStart + strlen(weaponPrefix) : szWeaponName;

    return extractedWeaponName;
}


void F::RAGE::impl::AutomaticStop(C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, CUserCmd* cmd, stop_mode mode)
{
    if (!pLocal || !weapon)
        return;

    if (!pLocal->IsValidMoveType() || !cheat->onground)
        return;

    if (rage_data.rage_hitchance <= 0.f)
        return;

    auto movesys = pLocal->m_pMovementServices();
    if (!movesys) {
        return;
    }
    auto weapon_data = weapon->datawep();

    if (!weapon_data)
        return;

    if ((weapon->m_zoomLevel() == 0 && weapon_data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE))
        return;

    if (!m_stop)
        return;

    if (!rage_data.auto_stop)
        return;

    float m_max_speed = weapon->get_max_speed();
    Vector_t velocity = pLocal->m_vecVelocity();


    const char* extractedWeaponName = GetExtractedWeaponName(weapon);

    bool has_autos = strcmp(extractedWeaponName, CS_XOR("scar20")) == 0 || strcmp(extractedWeaponName, CS_XOR("g3sg1")) == 0;
    if (!cheat->canShot && !has_autos && weapon_data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE)
        return;

    // set z to 0
    velocity.z = 0.f;

    float speed = velocity.Length2D();

    // we are pressing move keys
    bool pressing_move_keys = (cmd->m_nButtons.m_nValue & IN_FORWARD || cmd->m_nButtons.m_nValue & IN_MOVELEFT ||
        cmd->m_nButtons.m_nValue & IN_BACK || cmd->m_nButtons.m_nValue & IN_MOVERIGHT);

    cmd->m_nButtons.m_nValue &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK | IN_SPEED);

    bool fullstop = true;
    float addspeed, accelspeed;
    auto accel = I::Cvar->Find(FNV1A::Hash(CS_XOR("sv_accelerate")))->value.fl;
    float max_accurate_speed = (m_max_speed * 0.33000001);
    bool inaccurate_speed = speed > max_accurate_speed;
    float surfaceFriction = pLocal->m_pMovementServices()->m_flSurfaceFriction();
    float wish_speed = sqrt(cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove * cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove + cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove * cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove);
 

    Vector_t angle;
    MATH::vec_angles(velocity, &angle);
    angle.y = cmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue.y - angle.y;

    Vector_t direction;
    AngleVectors(angle, &direction);

    Vector_t stop = direction * -speed;

    cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = 0;
    cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = 0;
    if (speed > max_accurate_speed) {

        cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = 0;
        cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = 0;
        cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = stop.x;
        cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = stop.y;
    }
    else {
        float sidemove = cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove;
        float forwardmove = cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove;
        float upmove = cmd->m_csgoUserCmd.m_pBaseCmd->m_flUpMove;

        float move_speed = std::sqrt(std::pow(sidemove, 2) + std::pow(forwardmove, 2) + std::pow(upmove, 2));

        if (move_speed > max_accurate_speed) {
            float desaccel = max_accurate_speed / move_speed;
            cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove *= desaccel;
            cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove *= desaccel;
        }
        else
        {
            cmd->m_csgoUserCmd.m_pBaseCmd->m_flForwardMove = 0;
            cmd->m_csgoUserCmd.m_pBaseCmd->m_flSideMove = 0;
        }
    }

}/*
void F::RAGE::impl::AutomaticStop(C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, CUserCmd* cmd, stop_mode mode)
{
    if (!pLocal || !weapon)
        return;

    if (!pLocal->IsValidMoveType() || !cheat->onground) {
        return;
    }

    if (rage_data.rage_hitchance <= 0.f)
        return;

    auto movesys = pLocal->m_pMovementServices();
    if (!movesys) {
        return;
    }
    auto weapon_data = weapon->datawep();

    if (!weapon_data)
        return;

    if ((weapon->m_zoomLevel() == 0 && weapon_data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE))
        return;

    const char* extractedWeaponName = GetExtractedWeaponName(weapon);

    bool has_autos = strcmp(extractedWeaponName, CS_XOR("scar20")) == 0 || strcmp(extractedWeaponName, CS_XOR("g3sg1")) == 0;
    bool has_snipers = strcmp(extractedWeaponName, CS_XOR("awp")) == 0 || strcmp(extractedWeaponName, CS_XOR("ssg08")) == 0;

    auto m_max_cycle = weapon->m_weaponMode() == CSWeaponMode::Primary_Mode ? weapon_data->m_flCycleTime().flValue[0] : weapon_data->m_flCycleTime().flValue[1];

    const float max_speed = weapon->m_weaponMode() == CSWeaponMode::Primary_Mode ? weapon_data->m_flMaxSpeed().flValue[0] : weapon_data->m_flMaxSpeed().flValue[1];
    const float max_accurate_speed_percentage = 0.34f;  // Adjust the percentage as needed
    const float max_accurate_speed = max_speed * max_accurate_speed_percentage;
    const float cur_speed = pLocal->m_vecVelocity().Length2D();

    if (mode == slow) {
        if (has_autos) {
            if (pLocal->m_vecVelocity().Length2D() < 5.f)
                return;
        }
        else {
            if (!cheat->canShot)
                return;

            if (has_snipers) {
                if (pLocal->m_vecVelocity().Length2D() < 5.f)
                    return;
            }
            else {
                if (pLocal->m_vecVelocity().Length2D() < 15.f)
                    return;
            }

        }
        if (m_stop) {
            cmd->nButtons.nValue &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK | IN_SPEED);

            // get velocity data
            static float vel = pLocal->m_vecVelocity().length();

            bool vel_accurate = (cur_speed <= max_accurate_speed);
            // if we're inside limit accurate speed maintain velocity
            if (vel_accurate) {
                int a = MATH::fnRandomInt(0, 1);

                // randomly switch mode
                if (a == 0) {
                    QAngle_t resist_angle;
                    MATH::VectorAngless((pLocal->m_vecVelocity() * -1), resist_angle);
                    resist_angle.y = cmd->pBaseCmd->pCmdView->angValue.y - resist_angle.y;

                    Vector_t resistance = Vector_t(0, 0, 0);
                    MATH::anglevectors(resist_angle, &resistance);

                    // set movement
                    cmd->pBaseCmd->flForwardMove = std::clamp(resist_angle.x, -450.f, 450.f);
                    cmd->pBaseCmd->flSideMove = std::clamp(resist_angle.y, -450.f, 450.f);
                }
                else {
                    // force movement
                    cmd->pBaseCmd->flSideMove = 0.f;
                    cmd->pBaseCmd->flForwardMove = 0.f;

                    // clamp movement
                    cmd->pBaseCmd->flSideMove = std::clamp(cmd->pBaseCmd->flSideMove, -450.f, 450.f);
                    cmd->pBaseCmd->flForwardMove = std::clamp(cmd->pBaseCmd->flForwardMove, -450.f, 450.f);
                }

                // prevent we autostopping but unable to shoot due to wrong accurate speed
                // full stop imediatly otherwise we delayshooting rn
                if (pLocal->GetShotsFired() <= 0 && !has_autos) {
                    cmd->nButtons.nValue &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK | IN_SPEED);
                    // force movement
                    cmd->pBaseCmd->flSideMove = 0.f;
                    cmd->pBaseCmd->flForwardMove = 0.f;
                    cmd->nButtons.nValue &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK | IN_SPEED);

                    // clamp movement
                    cmd->pBaseCmd->flSideMove = std::clamp(cmd->pBaseCmd->flSideMove, -450.f, 450.f);
                    cmd->pBaseCmd->flForwardMove = std::clamp(cmd->pBaseCmd->flForwardMove, -450.f, 450.f);
                    return;
                }
            } // just slowly resist angle at high velocity then maintain the speed
            else {
                cmd->pBaseCmd->flSideMove = 0.f;
                cmd->pBaseCmd->flForwardMove = 0.f;
                QAngle_t resist_angle;
                MATH::VectorAngless((pLocal->m_vecVelocity() * -1), resist_angle);
                resist_angle.y = cmd->pBaseCmd->pCmdView->angValue.y - resist_angle.y;
                Vector_t resistance = Vector_t(0, 0, 0);
                MATH::anglevectors(resist_angle, &resistance);
                // set movement
                cmd->pBaseCmd->flForwardMove = std::clamp(resist_angle.x, -450.f, 450.f);
                cmd->pBaseCmd->flSideMove = std::clamp(resist_angle.y, -450.f, 450.f);
            }
            m_stop = false;
        }
    }
    // just full stop
    else if (mode == early && m_stop) {
        if (!weapon_data->m_bIsFullAuto() && !cheat->canShot)
            return;

        if (!weapon_data->m_bIsFullAuto()) {

            cmd->pBaseCmd->flForwardMove = 0.f;
            cmd->pBaseCmd->flSideMove = 0.f;
            QAngle_t resist_angle;
            MATH::VectorAngless((pLocal->m_vecVelocity() * -1), resist_angle);
            resist_angle.y = cmd->pBaseCmd->pCmdView->angValue.y - resist_angle.y;

            Vector_t resistance = Vector_t(0, 0, 0);
            MATH::anglevectors(resist_angle, &resistance);

            // set movement
            cmd->pBaseCmd->flForwardMove = std::clamp(resist_angle.x, -450.f, 450.f);
            cmd->pBaseCmd->flSideMove = std::clamp(resist_angle.y, -450.f, 450.f);
        }

        cmd->pBaseCmd->flForwardMove = 0.f;
        cmd->pBaseCmd->flSideMove = 0.f;
        m_stop = false;
    }

}*/
static bool compare_targets(const scanned_target& first, const scanned_target& second)
{
    // higher damage is preferred
    if (first.damage > second.damage)
        return true;
    else if (first.damage < second.damage)
        return false;

    // lower distance is preferred if damages are equal
    return first.distance < second.distance;
}

void F::RAGE::impl::SortTarget()
{
    auto validate_target = [&](scanned_target* a, scanned_target* b) -> bool {
        if (!a || !b)
            goto fuck_yeah;


        if (last_target != nullptr && last_target->GetHealth() <= 0 && a->record->GetIdentity()->GetIndex() != last_target->GetIdentity()->GetIndex() && last_target->GetIdentity()->GetIndex() <= 64) {
            return true;
        }

        if (a->damage == b->damage)
            return a->distance < b->distance;

        return a->damage > b->damage;

    fuck_yeah:
        // this might not make sense to you, but it actually does.
        return (a != nullptr || (a != nullptr && b != nullptr && a == b)) ? true : false;
    };

    for (auto& data : scanned_targets) {

        if (data.damage < 1.f)
            continue;

        if (!final_target.record) {
            final_target = data;

            // we only have one entry (target)? let's skip target selection..
            if (scanned_targets.size() == 1)
                break;
            else
                continue;
        }

        if (last_target != nullptr && last_target->GetHealth() <= 0 && data.record->GetIdentity()->GetIndex() != last_target->GetIdentity()->GetIndex()) {
            continue;
        }

        // sort our target based on our conditions
        if (validate_target(&data, &final_target)) {
            final_target = data;
            continue;
        }
    }

}
float F::RAGE::impl::ScaleDamage(C_CSPlayerPawn* target, C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, Vector_t aim_point, float& dmg, bool& canHit)
{
    if (!pLocal || !weapon || !target)
        return 0.f;

    if (pLocal->GetHealth() <= 0)
        return 0.f;

    auto vdata = weapon->datawep();
    if (!vdata)
        return 0.f;

    auto entity = I::GameResourceService->pGameEntitySystem->Get(target->GetRefEHandle());
    if (!entity)
        return 0.f;

    auto localent = I::GameResourceService->pGameEntitySystem->Get(pLocal->GetRefEHandle());
    if (!localent)
        return 0.f;

    float damage = 0.f;
    F::AUTOWALL::c_auto_wall::data_t data;
    F::AUTOWALL::g_auto_wall->pen(data, pLocal->GetEyePosition(), aim_point, entity, localent, pLocal, target, vdata, damage, canHit);
    return data.m_can_hit ? data.m_dmg : 0.f;
}

void F::RAGE::impl::AjustRecoil(C_CSPlayerPawn* player, CUserCmd* pCmd) {
    if (!player || !pCmd) return;

}

void F::RAGE::impl::Run(C_CSPlayerPawn* pLocal, CCSGOInput* pInput, CUserCmd* cmd) {


    if (!C_GET(bool, Vars.rage_enable))
        return;

    if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
        return;

    if (!SDK::LocalController || !SDK::LocalPawn)
        return;

    // sanity check
    if (!pLocal || pLocal->GetHealth() <= 0) {
        last_target = nullptr;
        return;
    }

    if (!pInput || !cmd)
        return;

    auto pCmd = cmd->m_csgoUserCmd.m_pBaseCmd;
    if (!pCmd)
        return;

    rage->Reset(reset::aimbot);

    // setup menu adaptive weapon with rage data
    rage->SetupAdaptiveWeapon(pLocal);

    // reset subtick data
    sub_tick_data.reset();

    // reset hitchance data
    rage_data.accurate = false;

    // no ammo or not valid weapon 
    if (!rage->Ready(pLocal))
        return;

    VIRTUALIZER_TIGER_WHITE_START

        // hitbox menu selection 
        rage->Scan();

    // store targets in server
    rage->SetupTarget(pLocal);

    // keep scanned targets getting updated 
    scanned_targets.clear();

    // scan & select them based on our conditions
    rage->ScanTarget(pLocal, cmd, pCmd->m_pViewangles->m_angValue);

    if (scanned_targets.empty())
        return;

    auto weapon_data = pLocal->ActiveWeapon();
    if (!weapon_data)
        return;

    auto vdata = weapon_data->datawep();
    if (!vdata)
        return;

    if (last_target != nullptr && last_target->GetHealth() <= 0) {
        last_target = nullptr;
    }

    // select best target
    rage->SortTarget();

    if (!(final_target.data.valid()))
        return;

    auto best_target = final_target;

    Hitchance(pLocal, best_target.record, weapon_data, best_target.data.point.pointangle);
    m_stop = cheat->onground && !rage_data.accurate;

    bool can_scope = weapon_data->m_zoomLevel() == 0 && weapon_data->datawep()->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE;
    if (can_scope && !rage_data.rapid_fire) {
        cmd->m_nButtons.m_nValue |= IN_SECOND_ATTACK;
    }

    rage->AutomaticStop(pLocal, weapon_data, cmd, slow);

    // calculate aimpunch & compensate it 
    static auto prev = QAngle_t(0.f, 0.f, 0.f);
    auto cache = pLocal->m_aimPunchCache();
    auto pred_punch = cache.m_Data[cache.m_Size - 1];
    auto delta = prev - pred_punch * 2.f;
    if (cache.m_Size > 0 && cache.m_Size <= 0xFFFF) {
        pred_punch = cache.m_Data[cache.m_Size - 1];
        prev = pred_punch;
    }

    const float time_per_tick = I::GlobalVars->flIntervalPerTick;
    const float best_time = final_target.record->m_flSimulationTime();
    const float best_tick_time = best_time / time_per_tick;
    int best_tick = (int)std::floor(best_tick_time);
    const float best_tick_fraction = best_tick_time - best_tick;

    best_tick -= 1; // FIXME: engine prediction?
    // store best_point with compensated aimpunch
    auto best_point = final_target.data.point.angle + delta * 2.f;

    /*  for (int i = 0; i < cmd->m_csgoUserCmd.m_inputHistoryField.m_pRep->m_nAllocatedSize; i++)
      {
          CCSGOInputHistoryEntryPB* pInputEntry = cmd->GetInputHistoryEntry(i);
          if (pInputEntry == nullptr)
              continue;

          if (pInputEntry->m_pViewCmd == nullptr)
              continue;

          pInputEntry->m_pViewCmd->m_angValue = best_point;
      }
    */

     cmd->SetSubTickAngle(best_point);
    if (cheat->canShot) {
        if (rage_data.rapid_fire && rage_data.accurate) {
            sub_tick_data.command = command_msg::rapid_fire;
            sub_tick_data.best_point = best_point;
        }
        else
        {
            if (rage_data.accurate) {
                sub_tick_data.command = command_msg::silent;
                sub_tick_data.best_point = best_point;

                // our InputMessage set our predicted subtick viewangles to best point we ready to shoot
                cmd->AdjustAttackStartIndex(pCmd->m_nTickCount - 1);
                cmd->m_nButtons.m_nValue |= IN_ATTACK;
            }
            /*   else if (rage_data.rapid_fire && cheat->canShot) {
                   cmd->m_csgoUserCmd.m_nAttack1StartHhistoryIndex = pCmd->m_nTickCount - 1;

                   if (!(cmd->m_nButtons.m_nValue & IN_ATTACK)) {
                       L_PRINT(LOG_INFO) << "shooting";
                       cmd->m_nButtons.m_nValue |= IN_ATTACK;
                   }
                   L_PRINT(LOG_INFO) << "on rapid fire";

                   for (int i = 0; i < cmd->m_csgoUserCmd.m_inputHistoryField.m_pRep->m_nAllocatedSize; i++)
                   {
                       CCSGOInputHistoryEntryPB* pInputEntry = cmd->GetInputHistoryEntry(i);
                       if (pInputEntry == nullptr)
                           continue;
                       if (pInputEntry->m_pViewCmd == nullptr)
                           continue;


                       pInputEntry->m_nPlayerTickCount = 0;
                       L_PRINT(LOG_INFO) << "tick:" << pInputEntry->m_nPlayerTickCount;

                   }
            */
        }
    } // run rapid fire on ( awp, scout, scar ) regeardless if we can shoot or not to avoid delays and shit.
    else  if (rage_data.accurate && rage_data.rapid_fire && weapon_data->datawep()->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE) {
        sub_tick_data.command = command_msg::rapid_fire;
        sub_tick_data.best_point = best_point;
    }
    else
        sub_tick_data.command = command_msg::none;


    last_target = final_target.record;

    VIRTUALIZER_MUTATE_ONLY_END;
}

void F::RAGE::impl::SetupAdaptiveWeapon(C_CSPlayerPawn* pLocal) {
    if (!C_GET(bool, Vars.rage_enable))
        return;

    if (!pLocal)
        return;

    if (pLocal->GetHealth() <= 0)
        return;

 
    auto ActiveWeapon = pLocal->ActiveWeapon();
    if (!ActiveWeapon)
        return;

    auto data = ActiveWeapon->datawep();
    if (!data)
        return;

    if (!ActiveWeapon->IsWeapon())
        return;

    const char* extractedWeaponName = GetExtractedWeaponName(ActiveWeapon);

    bool has_awp = strcmp(extractedWeaponName, CS_XOR("awp")) == 0;
    bool has_heavy_pistols = strcmp(extractedWeaponName, CS_XOR("revolver")) == 0 || strcmp(extractedWeaponName, CS_XOR("deagle")) == 0;
    bool has_scout = strcmp(extractedWeaponName, CS_XOR("ssg08")) == 0;
    rage_data.rage_enable = C_GET(bool, Vars.rage_enable);

    if (has_awp) {
        rage_data.minimum_damage = C_GET_ARRAY(int, 7, Vars.rage_minimum_damage, 6);
        rage_data.rage_hitchance = C_GET_ARRAY(int, 7, Vars.rage_minimum_hitchance, 6);
        rage_data.auto_stop = C_GET_ARRAY(bool, 7, Vars.rage_auto_stop, 6);
        rage_data.penetration = C_GET_ARRAY(bool, 7, Vars.rage_penetration, 6);
        rage_data.rage_target_select = C_GET_ARRAY(int, 7, Vars.rage_target_select, 6);
        rage_data.hitbox_head = C_GET_ARRAY(bool, 7, Vars.hitbox_head, 6);
        rage_data.hitbox_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_chest, 6);
        rage_data.hitbox_stomach = C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 6);
        rage_data.hitbox_legs = C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 6);
        rage_data.hitbox_feets = C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 6);
        rage_data.rapid_fire = C_GET_ARRAY(bool, 7, Vars.rapid_fire, 6);
    }
    else if (has_scout) {
        rage_data.minimum_damage = C_GET_ARRAY(int, 7, Vars.rage_minimum_damage, 5);
        rage_data.rage_hitchance = C_GET_ARRAY(int, 7, Vars.rage_minimum_hitchance, 5);
        rage_data.auto_stop = C_GET_ARRAY(bool, 7, Vars.rage_auto_stop, 5);
        rage_data.penetration = C_GET_ARRAY(bool, 7, Vars.rage_penetration, 5);
        rage_data.rage_target_select = C_GET_ARRAY(int, 7, Vars.rage_target_select, 5);
        rage_data.remove_weapon_accuracy_spread = C_GET_ARRAY(bool, 7, Vars.remove_weapon_accuracy_spread, 5);
        rage_data.hitbox_head = C_GET_ARRAY(bool, 7, Vars.hitbox_head, 5);
        rage_data.hitbox_neck = C_GET_ARRAY(bool, 7, Vars.hitbox_neck, 5);
        rage_data.hitbox_uppeer_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 5);
        rage_data.hitbox_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_chest, 5);
        rage_data.hitbox_stomach = C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 5);
        rage_data.hitbox_legs = C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 5);
        rage_data.hitbox_feets = C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 5);
        rage_data.rapid_fire = C_GET_ARRAY(bool, 7, Vars.rapid_fire, 5);

    }
    else if (has_heavy_pistols) {
        rage_data.minimum_damage = C_GET_ARRAY(int, 7, Vars.rage_minimum_damage, 2);
        rage_data.rage_hitchance = C_GET_ARRAY(int, 7, Vars.rage_minimum_hitchance, 2);
        rage_data.auto_stop = C_GET_ARRAY(bool, 7, Vars.rage_auto_stop, 2);
        rage_data.penetration = C_GET_ARRAY(bool, 7, Vars.rage_penetration, 2);
        rage_data.rage_target_select = C_GET_ARRAY(int, 7, Vars.rage_target_select, 2);
        rage_data.remove_weapon_accuracy_spread = C_GET_ARRAY(bool, 7, Vars.remove_weapon_accuracy_spread, 2);
        rage_data.hitbox_head = C_GET_ARRAY(bool, 7, Vars.hitbox_head, 2);
        rage_data.hitbox_neck = C_GET_ARRAY(bool, 7, Vars.hitbox_neck, 2);
        rage_data.hitbox_uppeer_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 2);
        rage_data.hitbox_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_chest, 2);
        rage_data.hitbox_stomach = C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 2);
        rage_data.hitbox_legs = C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 2);
        rage_data.hitbox_feets = C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 2);
        rage_data.rapid_fire = C_GET_ARRAY(bool, 7, Vars.rapid_fire, 2);

    }
    else if (data->m_WeaponType() == WEAPONTYPE_PISTOL && !has_heavy_pistols) {
        rage_data.minimum_damage = C_GET_ARRAY(int, 7, Vars.rage_minimum_damage, 1);
        rage_data.rage_hitchance = C_GET_ARRAY(int, 7, Vars.rage_minimum_hitchance, 1);
        rage_data.auto_stop = C_GET_ARRAY(bool, 7, Vars.rage_auto_stop, 1);
        rage_data.penetration = C_GET_ARRAY(bool, 7, Vars.rage_penetration, 1);
        rage_data.rage_target_select = C_GET_ARRAY(int, 7, Vars.rage_target_select, 1);
        rage_data.hitbox_head = C_GET(bool, 7, Vars.hitbox_head, 1);
        rage_data.hitbox_neck = C_GET(bool, 7, Vars.hitbox_neck, 1);
        rage_data.hitbox_uppeer_chest = C_GET(bool, 7, Vars.hitbox_uppeer_chest, 1);
        rage_data.hitbox_chest = C_GET(bool, 7, Vars.hitbox_chest, 1);
        rage_data.hitbox_stomach = C_GET(bool, 7, Vars.hitbox_stomach, 1);
        rage_data.hitbox_legs = C_GET(bool, 7, Vars.hitbox_legs, 1);
        rage_data.hitbox_feets = C_GET(bool, 7, Vars.hitbox_feet, 1);
        rage_data.rapid_fire = C_GET(bool, 7, Vars.rapid_fire, 1);
    }
    else if (data->m_WeaponType() == WEAPONTYPE_MACHINEGUN) {
        rage_data.minimum_damage = C_GET_ARRAY(int, 7, Vars.rage_minimum_damage, 3);
        rage_data.rage_hitchance = C_GET_ARRAY(int, 7, Vars.rage_minimum_hitchance, 3);
        rage_data.auto_stop = C_GET_ARRAY(bool, 7, Vars.rage_auto_stop, 3);
        rage_data.penetration = C_GET_ARRAY(bool, 7, Vars.rage_penetration, 3);
        rage_data.rage_target_select = C_GET_ARRAY(int, 7, Vars.rage_target_select, 3); 
        rage_data.hitbox_head = C_GET_ARRAY(bool, 7, Vars.hitbox_head, 3);
        rage_data.hitbox_neck = C_GET_ARRAY(bool, 7, Vars.hitbox_neck, 3);
        rage_data.hitbox_uppeer_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 3);
        rage_data.hitbox_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_chest, 3);
        rage_data.hitbox_stomach = C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 3);
        rage_data.hitbox_legs = C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 3);
        rage_data.hitbox_feets = C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 3);
        rage_data.rapid_fire = C_GET_ARRAY(bool, 7, Vars.rapid_fire, 3);

    }
    else if (data->m_WeaponType() == WEAPONTYPE_RIFLE) {
            rage_data.minimum_damage = C_GET_ARRAY(int, 7, Vars.rage_minimum_damage, 3);
            rage_data.rage_hitchance = C_GET_ARRAY(int, 7, Vars.rage_minimum_hitchance, 3);
            rage_data.auto_stop = C_GET_ARRAY(bool, 7, Vars.rage_auto_stop, 3);
            rage_data.penetration = C_GET_ARRAY(bool, 7, Vars.rage_penetration, 3);
            rage_data.rage_target_select = C_GET_ARRAY(int, 7, Vars.rage_target_select, 3);
            rage_data.hitbox_head = C_GET_ARRAY(bool, 7, Vars.hitbox_head, 3);
            rage_data.hitbox_neck = C_GET_ARRAY(bool, 7, Vars.hitbox_neck, 3);
            rage_data.hitbox_uppeer_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 3);
            rage_data.hitbox_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_chest, 3);
            rage_data.hitbox_stomach = C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 3);
            rage_data.hitbox_legs = C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 3);
            rage_data.hitbox_feets = C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 3);
            rage_data.rapid_fire = C_GET_ARRAY(bool, 7, Vars.rapid_fire, 3);
    }
    else if (data->m_WeaponType() == WEAPONTYPE_SNIPER_RIFLE){
            rage_data.minimum_damage = C_GET_ARRAY(int, 7, Vars.rage_minimum_damage, 4);
            rage_data.rage_hitchance = C_GET_ARRAY(int, 7, Vars.rage_minimum_hitchance, 4);
            rage_data.auto_stop = C_GET_ARRAY(bool, 7, Vars.rage_auto_stop, 4);
            rage_data.penetration = C_GET_ARRAY(bool, 7, Vars.rage_penetration, 4);
            rage_data.rage_target_select = C_GET_ARRAY(int, 7, Vars.rage_target_select, 4);
            rage_data.hitbox_head = C_GET_ARRAY(bool, 7, Vars.hitbox_head, 4);
            rage_data.hitbox_neck = C_GET_ARRAY(bool, 7, Vars.hitbox_neck, 4);
            rage_data.hitbox_uppeer_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_uppeer_chest, 4);
            rage_data.hitbox_chest = C_GET_ARRAY(bool, 7, Vars.hitbox_chest, 4);
            rage_data.hitbox_stomach = C_GET_ARRAY(bool, 7, Vars.hitbox_stomach, 4);
            rage_data.hitbox_legs = C_GET_ARRAY(bool, 7, Vars.hitbox_legs, 4);
            rage_data.hitbox_feets = C_GET_ARRAY(bool, 7, Vars.hitbox_feet, 4);
            rage_data.rapid_fire = C_GET_ARRAY(bool, 7, Vars.rapid_fire, 4);
    }

}
