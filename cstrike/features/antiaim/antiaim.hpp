#pragma once
#include "../../sdk/datatypes/qangle.h"
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../core/sdk.h"
#include "../../core/interfaces.h"
#include "../../sdk/datatypes/usercmd.h"

namespace F::ANTIAIM
{
	inline QAngle_t angStoredViewBackup{};
	void RunAA(CUserCmd* pCmd);
}