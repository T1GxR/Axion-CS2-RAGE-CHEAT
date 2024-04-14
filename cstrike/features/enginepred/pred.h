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
class CCSWeaponBaseVData;
namespace F::PREDICTION
{
	class impl {
	public:

		void start(CUserCmd* cmd);

		void end();

		std::uint32_t Flags();

	};
	const auto prediction = std::make_unique<impl>();
}
