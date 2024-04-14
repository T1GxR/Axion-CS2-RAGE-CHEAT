#include "penetration.h"
#include "../../core/variables.h"
#include "../../sdk/datatypes/usercmd.h"
#include "../../core/sdk.h"
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/iglobalvars.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/datatypes/qangle.h"
#include "../../sdk/datatypes/vector.h"
#include "../../sdk/entity_handle.h"
#include "../cstrike/sdk/interfaces/ienginecvar.h"

#include <mutex>
#include <array>
#include "../cstrike/sdk/interfaces/itrace.h"
static constexpr std::uint32_t PENMASK = 0x1C300Bu; // mask_shot_hull | contents_hitbox?

namespace F::AUTOWALL {

  

    void F::AUTOWALL::c_auto_wall::pen(data_t& data, const Vector_t local_pos, const Vector_t target_pos, C_BaseEntity* target, C_BaseEntity* local, C_CSPlayerPawn* localpawn, C_CSPlayerPawn* targetpawn,
        CCSWeaponBaseVData* wpn_data, float &dmg, bool &valid) {
        data.m_local = local;
        data.m_target = target;
        data.m_local_pawn = localpawn;
        data.m_target_pawn = targetpawn;
        data.m_wpn_data = wpn_data;
        data.m_pos.at(F::AUTOWALL::c_auto_wall::data_t::e_pos::e_local) = local_pos;
        data.m_pos.at(F::AUTOWALL::c_auto_wall::data_t::e_pos::e_target) = target_pos;
        FireBullet(data, data.m_dmg, data.m_can_hit);
        
    }

    void F::AUTOWALL::c_auto_wall::ScaleDamage(data_t& data, const int hitgroup, C_CSPlayerPawn* player)
    {
        if (!player)
            return;

        auto WeaponServices = player->GetItemServices();
        if (!WeaponServices)
            return;

        // ida: server.dll; 80 78 42 00 74 08 F3 0F 59 35 ?? ?? ?? ?? 80 BE 04 0D 00 00 00
        static CConVar* mp_damage_scale_ct_head = I::Cvar->Find(
            FNV1A::HashConst("mp_damage_scale_ct_head")),
            * mp_damage_scale_t_head = I::Cvar->Find(
                FNV1A::HashConst("mp_damage_scale_t_head")),
            * mp_damage_scale_ct_body = I::Cvar->Find(
                FNV1A::HashConst("mp_damage_scale_ct_body")),
            * mp_damage_scale_t_body = I::Cvar->Find(
                FNV1A::HashConst("mp_damage_scale_t_body"));

        const auto damage_scale_ct_head = mp_damage_scale_ct_head->value.fl,
            damage_scale_t_head = mp_damage_scale_t_head->value.fl,
            damage_scale_ct_body = mp_damage_scale_ct_body->value.fl,
            damage_scale_t_body = mp_damage_scale_t_body->value.fl;

        const bool is_ct = player->GetTeam() == 3, is_t = player->GetTeam() == 2;

        float head_damage_scale = is_ct ? damage_scale_ct_head : is_t ? damage_scale_t_head : 1.0f;
        const float body_damage_scale = is_ct ? damage_scale_ct_body : is_t ? damage_scale_t_body : 1.0f;

        // william: magic values u can see here: ida: server.dll; F3 0F 10 35 ?? ?? ?? ?? 0F 29 7C 24 30 44 0F 29 44 24
        // xref: mp_heavybot_damage_reduction_scale
        if (WeaponServices->m_bHasHeavyArmor()) {
            head_damage_scale *= 0.5f;
        }

        // todo: mb missed some hitgroups, anyway this is a calculation of all important hitgroups
        switch (hitgroup) {
        case HitGroup_t::HITGROUP_HEAD:
            data.m_dmg *= data.m_wpn_data->m_flHeadshotMultiplier() * head_damage_scale;
            break;
        case HitGroup_t::HITGROUP_CHEST:
        case HitGroup_t::HITGROUP_LEFTARM:
        case HitGroup_t::HITGROUP_RIGHTARM:
        case HitGroup_t::HITGROUP_NECK:
            data.m_dmg *= body_damage_scale;
            break;
        case HitGroup_t::HITGROUP_STOMACH:
            data.m_dmg *= 1.25f * body_damage_scale;
            break;
        case HitGroup_t::HITGROUP_LEFTLEG:
        case HitGroup_t::HITGROUP_RIGHTLEG:
            data.m_dmg *= 0.75f * body_damage_scale;
            break;
        default:
            break;
        }

        if (!player->hasArmour(hitgroup))
            return;

        float heavy_armor_bonus = 1.0f, armor_bonus = 0.5f, armor_ratio = data.m_wpn_data->m_flArmorRatio() * 0.5f;

        if (WeaponServices->m_bHasHeavyArmor()) {
            heavy_armor_bonus = 0.25f;
            armor_bonus = 0.33f;
            armor_ratio *= 0.20f;
        }

        float damage_to_health = data.m_dmg * armor_ratio;
        const float damage_to_armor = (data.m_dmg - damage_to_health) * (heavy_armor_bonus * armor_bonus);

        if (damage_to_armor > static_cast<float>(player->GetArmorValue())) {
            damage_to_health = data.m_dmg - static_cast<float>(player->GetArmorValue()) / armor_bonus;
        }

        data.m_dmg = damage_to_health;
    }

    
    bool F::AUTOWALL::c_auto_wall::FireBullet(data_t& data, float &dmg, bool &valid)
    {
     //   L_PRINT(LOG_INFO) << "0";
        CS_ASSERT(data.m_local != nullptr || data.m_target != nullptr || data.m_wpn_data != nullptr || data.m_local_pawn != nullptr || data.m_target_pawn != nullptr);
        if (!data.m_local || !data.m_target || !data.m_wpn_data)
            return false;
    //    L_PRINT(LOG_INFO) << "1";

        trace_data_t trace_data = { };
        trace_data.m_arr_pointer = &trace_data.m_arr;
        void* data_pointer = &trace_data;
        //L_PRINT(LOG_INFO) << "created trace_data | trace_data_pointer:" << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(data_pointer);
        //L_PRINT(LOG_INFO) << "created trace_data | pointer:" << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(trace_data.m_arr_pointer);

        const Vector_t direction =
            data.m_pos.at(data_t::e_pos::e_target) - data.m_pos.at(data_t::e_pos::e_local),
            end_pos = direction * data.m_wpn_data->m_flRange();

        trace_filter_t filter = {};
        I::Trace->Init(filter, data.m_local_pawn,PENMASK,  3, 7);
        void* filter_pointer = &filter;
       /* L_PRINT(LOG_INFO) << "created filter_data | filter_pointer:" << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<uintptr_t>(filter_pointer);

        L_PRINT(LOG_INFO) << "creating trace | filter > layer:" << filter.layer << "mask:" << filter.trace_mask;
        L_PRINT(LOG_INFO) << "creating trace | endpos:" << end_pos;
        */
        I::Trace->CreateTrace(&trace_data, data.m_pos.at(data_t::e_pos::e_local), end_pos, filter, 4);

        struct handle_bullet_data_t {
            handle_bullet_data_t(const float dmg_mod, const float pen, const float range_mod, const float range,
                const int pen_count, const bool failed) :
                m_dmg(dmg_mod),
                m_pen(pen),
                m_range_mod(range_mod),
                m_range(range),
                m_pen_count(pen_count),
                m_failed(failed) {}

            float m_dmg{ }, m_pen{ }, m_range_mod{ }, m_range{ };
            int m_pen_count{ };
            bool m_failed{ };
        }

        handle_bullet_data(static_cast<float>(data.m_wpn_data->m_nDamage()), data.m_wpn_data->m_flPenetration(), data.m_wpn_data->m_flRange(),
            data.m_wpn_data->m_flRangeModifier(), 4, false);

      //  L_PRINT(LOG_INFO) << "Initialized handlebulletpen data";

        float corrected_dmg = static_cast<float>(data.m_wpn_data->m_nDamage());
        float flTraceLength = 0.f;
        auto flMaxRange = data.m_wpn_data->m_flRange();
        if (trace_data.m_num_update > 0) {
            for (int i{ }; i < trace_data.m_num_update; i++) {
                auto* value = reinterpret_cast<UpdateValueT* const>(
                    reinterpret_cast<std::uintptr_t>(trace_data.m_pointer_update_value)
                    + i * sizeof(UpdateValueT));

                game_trace_t game_trace = { };
                I::Trace->InitializeTraceInfo(&game_trace);
                I::Trace->get_trace_info(
                    &trace_data, &game_trace, 0.0f,
                    reinterpret_cast<void*>(
                        reinterpret_cast<std::uintptr_t>(trace_data.m_arr.data())
                        + sizeof(trace_arr_element_t) * (value->handleIdx & 0x7fffu)));
                /*
                L_PRINT(LOG_INFO) << "game_trace: entryindex:" << game_trace.HitEntity->GetRefEHandle().GetEntryIndex();
                L_PRINT(LOG_INFO) << "game_trace: m_target_pawn entryindex:" << data.m_target_pawn->GetRefEHandle().GetEntryIndex();
                L_PRINT(LOG_INFO) << "game_trace: m_target entryindex:" << data.m_target->GetRefEHandle().GetEntryIndex();
                */

                flMaxRange -= flTraceLength;

                // we didn't hit anything, stop tracing shoot
                if (game_trace.Fraction == 1.0f)
                {
                    break;
                }

                // calculate the damage based on the distance the bullet traveled
                flTraceLength += game_trace.Fraction * flMaxRange;
                corrected_dmg *= std::powf(data.m_wpn_data->m_flRangeModifier(), flTraceLength / 500.f);

                 // check is actually can shoot through
                 if (flTraceLength > 3000.f)
                     break;

                if (game_trace.HitEntity && game_trace.HitEntity->GetRefEHandle().GetEntryIndex() == data.m_target_pawn->GetRefEHandle().GetEntryIndex()) {
                    ScaleDamage2(game_trace.HitboxData->m_hitgroup, data.m_target_pawn, data.m_wpn_data->m_flArmorRatio(), data.m_wpn_data->m_flHeadshotMultiplier(), &corrected_dmg);
                    dmg = corrected_dmg;//data.m_dmg > 0.f ? data.m_dmg : handle_bullet_data.m_dmg;
                    valid = true;
                    return true;
                }
                else
                    valid = false;

   
                if (I::Trace->handle_bullet_penetration(&trace_data, &handle_bullet_data, value, false))                
                    return false;

                corrected_dmg = handle_bullet_data.m_dmg;   

            }
        }

        return false;
    }

    void F::AUTOWALL::c_auto_wall::ScaleDamage2(const int iHitGroup, C_CSPlayerPawn* pCSPlayer, const float flWeaponArmorRatio, const float flWeaponHeadShotMultiplier, float* pflDamageToScale)
    {
        // @ida CCSPlayer::TraceAttack(): server.dll -> "55 8B EC 83 E4 F8 81 EC ? ? ? ? 56 8B 75 08 57 8B F9 C6"
        auto WeaponServices = pCSPlayer->GetItemServices();
        if (!WeaponServices)
            return;

        const bool bHeavyArmor = WeaponServices->m_bHasHeavyArmor();

        // ida: server.dll; 80 78 42 00 74 08 F3 0F 59 35 ?? ?? ?? ?? 80 BE 04 0D 00 00 00
        static CConVar* mp_damage_scale_ct_head = I::Cvar->Find(
            FNV1A::HashConst("mp_damage_scale_ct_head")),
            * mp_damage_scale_t_head = I::Cvar->Find(
                FNV1A::HashConst("mp_damage_scale_t_head")),
            * mp_damage_scale_ct_body = I::Cvar->Find(
                FNV1A::HashConst("mp_damage_scale_ct_body")),
            * mp_damage_scale_t_body = I::Cvar->Find(
                FNV1A::HashConst("mp_damage_scale_t_body"));

        const auto damage_scale_ct_head = mp_damage_scale_ct_head->value.fl,
            damage_scale_t_head = mp_damage_scale_t_head->value.fl,
            damage_scale_ct_body = mp_damage_scale_ct_body->value.fl,
            damage_scale_t_body = mp_damage_scale_t_body->value.fl;

        float flHeadDamageScale = (pCSPlayer->GetTeam() == TEAM_CT ? damage_scale_ct_head : damage_scale_t_head);
        const float flBodyDamageScale = (pCSPlayer->GetTeam() == TEAM_CT ? damage_scale_ct_body : damage_scale_t_body);

        if (bHeavyArmor)            flHeadDamageScale *= 0.5f;

        switch (iHitGroup)
        {
        case HITGROUP_HEAD:
            *pflDamageToScale *= flWeaponHeadShotMultiplier * flHeadDamageScale;
            break;
        case HITGROUP_CHEST:
        case HITGROUP_LEFTARM:

        case HITGROUP_RIGHTARM:
        case HITGROUP_NECK:
            *pflDamageToScale *= flBodyDamageScale;
            break;
        case HITGROUP_STOMACH:
            *pflDamageToScale *= 1.25f * flBodyDamageScale;
            break;
        case HITGROUP_LEFTLEG:
        case HITGROUP_RIGHTLEG:
            *pflDamageToScale *= 0.75f * flBodyDamageScale;
            break;
        default:
            break;
        }

        if (pCSPlayer->hasArmour(iHitGroup))
        {
            // @ida CCSPlayer::OnTakeDamage(): server.dll -> "80 BF ? ? ? ? ? F3 0F 10 5C 24 ? F3 0F 10 35"

            const int iArmor = pCSPlayer->GetArmorValue();
            float flHeavyArmorBonus = 1.0f, flArmorBonus = 0.5f, flArmorRatio = flWeaponArmorRatio * 0.5f;

            if (bHeavyArmor)
            {
                flHeavyArmorBonus = 0.25f;
                flArmorBonus = 0.33f;
                flArmorRatio *= 0.20f;
            }

            float flDamageToHealth = *pflDamageToScale * flArmorRatio;
            if (const float flDamageToArmor = (*pflDamageToScale - flDamageToHealth) * (flHeavyArmorBonus * flArmorBonus); flDamageToArmor > static_cast<float>(iArmor))
                flDamageToHealth = *pflDamageToScale - static_cast<float>(iArmor) / flArmorBonus;

            *pflDamageToScale = flDamageToHealth;
        }
    }
   /* bool F::AUTOWALL::c_auto_wall::FireBullet(data_t& data, float& dmg, bool& valid)
    {
        CS_ASSERT(data.m_local != nullptr || data.m_target != nullptr || data.m_wpn_data != nullptr || data.m_local_pawn != nullptr || data.m_target_pawn != nullptr);
        if (!data.m_local || !data.m_target || !data.m_wpn_data)
            return false;

        trace_data_t trace_data = { };
        trace_data.m_arr_pointer = &trace_data.m_arr;

        Vector_t direction =
            data.m_pos.at(data_t::e_pos::e_target) - data.m_pos.at(data_t::e_pos::e_local),
            end_pos = direction * data.m_wpn_data->m_flRange();
     
        int pen_count = 4;
        float flTraceLength = 0.f;
        float flMaxRange = data.m_wpn_data->m_flRange();
        // set our current damage to what our gun's initial damage reports it will do
        data.m_dmg = static_cast<float>(data.m_wpn_data->m_nDamage());

        trace_filter_t filter = {};
        I::Trace->Init(filter, data.m_local_pawn, MASK_SHOT_HULL | CONTENTS_HITBOX, 3, 7);
        I::Trace->CreateTrace(&trace_data, data.m_pos.at(data_t::e_pos::e_local), end_pos, filter, 4);

        struct handle_bullet_data_t {
            handle_bullet_data_t(const float dmg_mod, const float pen, const float range_mod, const float range,
                const int pen_count, const bool failed) :
                m_dmg(dmg_mod),
                m_pen(pen),
                m_range_mod(range_mod),
                m_range(range),
                m_pen_count(pen_count),
                m_failed(failed) {}

            float m_dmg{ }, m_pen{ }, m_range_mod{ }, m_range{ };
            int m_pen_count{ };
            bool m_failed{ };
        }

        handle_bullet_data(static_cast<float>(data.m_wpn_data->m_nDamage()), data.m_wpn_data->m_flPenetration(), data.m_wpn_data->m_flRange(),
            data.m_wpn_data->m_flRangeModifier(), 4, false);

        if (trace_data.m_num_update > 0) {
            for (int i{ }; i < trace_data.m_num_update; i++) {
                auto* value = reinterpret_cast<UpdateValueT* const>(
                    reinterpret_cast<std::uintptr_t>(trace_data.m_pointer_update_value)
                    + i * sizeof(UpdateValueT));

                game_trace_t game_trace = { };
                I::Trace->InitializeTraceInfo(&game_trace);
                I::Trace->get_trace_info(
                    &trace_data, &game_trace, 0.0f,
                    reinterpret_cast<void*>(
                        reinterpret_cast<std::uintptr_t>(trace_data.m_arr.data())
                        + sizeof(trace_arr_element_t) * (value->handleIdx & 0x7fffu)));


                flMaxRange -= flTraceLength;

                game_trace_t trace = {};
                ray_t ray = {};

                I::Trace->TraceShape(ray, &data.m_pos.at(data_t::e_pos::e_local), &end_pos, filter, trace);
                I::Trace->ClipTraceToPlayers(data.m_pos.at(data_t::e_pos::e_local), end_pos, &filter, &trace, 0.F, 60.F, (1.F / (data.m_pos.at(data_t::e_pos::e_local) - end_pos).Length()) * (trace.m_end_pos - data.m_pos.at(data_t::e_pos::e_local)).Length());

                if (trace.Fraction == 1.f)
                    break;

                // calculate the damage based on the distance the bullet traveled
                flTraceLength += trace.Fraction * flMaxRange;
                data.m_dmg *= std::powf(data.m_wpn_data->m_flRangeModifier(), flTraceLength / 500.f);

                // check is actually can shoot through
                if (flTraceLength > 3000.f)
                    break;

                L_PRINT(LOG_INFO) << "idx:" << trace.HitEntity->GetRefEHandle().GetEntryIndex() << " dmg:" << data.m_dmg;

                if (trace.HitEntity->GetRefEHandle().GetEntryIndex() == data.m_target_pawn->GetRefEHandle().GetEntryIndex())
                {
                    ScaleDamage2(game_trace.HitboxData->m_hitgroup, data.m_target_pawn, data.m_wpn_data->m_flArmorRatio(), data.m_wpn_data->m_flHeadshotMultiplier(), &data.m_dmg);
                    L_PRINT(LOG_INFO) << "dmg:" << data.m_dmg;
                    return true;
                }

                // check if the bullet can no longer continue penetrating materials
                if (I::Trace->handle_bullet_penetration(&trace_data, &handle_bullet_data, value, false))
                    return false;

                pen_count = handle_bullet_data.m_pen_count;
                data.m_dmg = handle_bullet_data.m_dmg;
                    
            }
        }
        return false;
    }*/
    std::unique_ptr<c_auto_wall> g_auto_wall{};

}