#include "visuals.h"

// used: source sdk
#include "../sdk/interfaces/iengineclient.h"
#include "../sdk/entity.h"

// used: overlay
#include "visuals/overlay.h"
#include "visuals/chams.h"

#include "../core/sdk.h"

using namespace F;

bool F::VISUALS::Setup()
{
	if (!ams::chams->initialize())
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to initialize DrawObject chams | swapping to K3V");
		return true;
	}	
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_GREEN | LOG_COLOR_FORE_INTENSITY) << CS_XOR("chams initialization completed");

	
	return true;
}

bool F::VISUALS::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, material_data_t* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	return ams::chams->draw_object(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}
