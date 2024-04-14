#pragma once

#include "common.h"

class CUserCmd;
class CCSPlayerController;
class material_data_t;
class CCSGOInput;
namespace F
{
	bool Setup();
	void OnKeyBindUpdate();
	bool OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, material_data_t* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);
}
