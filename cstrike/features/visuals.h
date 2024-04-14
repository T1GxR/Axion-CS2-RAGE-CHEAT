#pragma once

class material_data_t;

namespace F::VISUALS
{
	bool Setup();
	void OnFrame(const int nStage);
	bool OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, material_data_t* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);
}
