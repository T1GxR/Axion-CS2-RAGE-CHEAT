#pragma once

#include "../../common.h"
#include <memory>
// used: draw system
#include "../../utilities/draw.h"
#include "../../sdk/datatypes/vector.h"
#include "../../sdk/datatypes/transform.h"
#include "../../sdk/datatypes/qangle.h"
#include "../cstrike/core/config.h"
#include <array>
class C_BaseEntity;
class CCSWeaponBaseVData;
class Vector_t;
class C_CSPlayerPawn;
class CCSPlayerController;
class CBaseHandle;
class CEntityInstance;
class CUserCmd;
class CBaseUserCmdPB;
class QAngle_t;
class IGameEvent;

namespace F::AUTOWALL {
    class c_auto_wall {      
    public:
        struct data_t {
            enum e_pos { e_local, e_target };
            data_t() = default;

            CS_INLINE data_t(const Vector_t local_pos, const Vector_t target_pos,
                C_BaseEntity* target,
                C_BaseEntity* local,
                C_CSPlayerPawn* localpawn, C_CSPlayerPawn* targetpawn,
                CCSWeaponBaseVData* wpn_data, const bool fire_bullet) noexcept :
                m_pos{ local_pos, target_pos },
                m_target(target),
                m_local(local),
                m_local_pawn(localpawn),
                m_target_pawn(targetpawn),
                m_wpn_data(wpn_data),
                m_can_hit(fire_bullet){}


            std::array< Vector_t, static_cast<int>(sizeof(e_pos)) > m_pos{ };
            C_BaseEntity* m_target{ }, * m_local{ };
            C_CSPlayerPawn* m_local_pawn{};
            C_CSPlayerPawn* m_target_pawn{};
            CCSWeaponBaseVData* m_wpn_data{ };

            bool m_can_hit{ };
            float m_dmg{ };
        } m_data;
        void pen(data_t& data, const Vector_t local_pos, const Vector_t target_pos, C_BaseEntity* target, C_BaseEntity* local, C_CSPlayerPawn* localpawn, C_CSPlayerPawn* targetpawn,
            CCSWeaponBaseVData* wpn_data, float &dmg, bool &valid);
        void ScaleDamage(data_t& data, const int hitgroup, C_CSPlayerPawn* entity);
        bool FireBullet(data_t& data, float &dmg, bool &valid);
        void ScaleDamage2(const int iHitGroup, C_CSPlayerPawn* pCSPlayer, const float flWeaponArmorRatio, const float flWeaponHeadShotMultiplier, float* pflDamageToScale);
    public:
        [[nodiscard]] CS_INLINE auto get_data(data_t& data) const noexcept { return data; }
    };

    extern std::unique_ptr<c_auto_wall> g_auto_wall;
}