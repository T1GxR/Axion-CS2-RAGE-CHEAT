#include "pred.h"
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
#include <mutex>
#include <array>
static float old_current_time = 0.f;
static float old_frame_time = 0.f;
static std::int32_t prediction_seed = 0;
static std::uint32_t predicted_flags = 0;

void F::PREDICTION::impl::start(CUserCmd* cmd)
{
    if (!SDK::LocalController || !SDK::LocalPawn) {
        return;
    }
    auto controller = SDK::LocalController;
    if (!controller)
        return;

    auto local = SDK::LocalPawn;
    if (!local)
        return;

    predicted_flags = local->GetFlags();

    // random_seed

    old_current_time = I::GlobalVars->flCurtime;
    old_frame_time = I::GlobalVars->flFrameTime;

   I::GlobalVars->flCurtime = controller->m_nTickBase() * I::GlobalVars->flIntervalPerTick;
   I::GlobalVars->flCurtime2 = controller->m_nTickBase() * I::GlobalVars->flIntervalPerTick;

   I::GlobalVars->flFrameTime = I::GlobalVars->flIntervalPerTick;
   I::GlobalVars->flFrameTime2 = I::GlobalVars->flIntervalPerTick;

}

void F::PREDICTION::impl::end()
{
    if (!SDK::LocalController || !SDK::LocalPawn) {
        return;
    }
    auto controller = SDK::LocalController;
    if (!controller)
        return;

    auto local = SDK::LocalPawn;
    if (!local)
        return;

    I::GlobalVars->flCurtime = old_current_time;
    I::GlobalVars->flCurtime2 = old_current_time;

    I::GlobalVars->flFrameTime = old_current_time;
    I::GlobalVars->flFrameTime2 = old_current_time;
}

std::uint32_t F::PREDICTION::impl::Flags()
{
    return predicted_flags;
}