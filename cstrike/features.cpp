#include "features.h"

// used: draw callbacks
#include "utilities/draw.h"
// used: notify
#include "utilities/notify.h"

// used: cheat variables
#include "core/variables.h"
// used: menu
#include "core/menu.h"

// used: features callbacks
#include "features/visuals.h"
#include "features/misc.h"
#include "features/legit/legit.h"
// used: interfaces
#include "core/interfaces.h"
#include "sdk/interfaces/iengineclient.h"
#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/datatypes/usercmd.h"
#include "sdk/entity.h"
#include <Windows.h>
#include "../cstrike/sdk/interfaces/iinputsystem.h"
#include "../cstrike/utilities/inputsystem.h"
#include "features/visuals/overlay.h"
#include "features/skins/skin_changer.hpp"

std::vector<KeyBind_t*> g_keybinds;
//[181.214.231.239:27042
bool F::Setup()
{
	if (!VISUALS::Setup())
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to setup visuals");
		return false;
	}
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_GREEN | LOG_COLOR_FORE_INTENSITY) << CS_XOR("features VISUALS completed");

	return true;
}

void F::OnKeyBindUpdate() {

	if (IPT::GetBindState(C_GET(KeyBind_t, Vars.OverlayKey)) || C_GET(KeyBind_t, Vars.OverlayKey).bEnable) {
		const KeyBind_t& overlayKeyBind = C_GET(KeyBind_t, Vars.OverlayKey);

		L_PRINT(LOG_INFO) << CS_XOR("[Keybind] GotBindState | key: ") <<
			overlayKeyBind.uKey << "| name: " <<
			overlayKeyBind.szName << "| states ";

		if (overlayKeyBind.nMode == EKeyBindMode::HOLD) {
			C_SET(bool, Vars.bVisualOverlay, true);
		}
		else if (overlayKeyBind.nMode == EKeyBindMode::TOGGLE) {
			C_SET(bool, Vars.bVisualOverlay, !C_GET(bool, Vars.bVisualOverlay));
		}
	}
}

bool F::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, material_data_t* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	return VISUALS::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}
