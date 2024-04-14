#pragma once

// used: [d3d] api
#include <d3d11.h>
#include <dxgi1_2.h>

// used: chookobject
#include "../utilities/detourhook.h"
#include "..\sdk\entity_handle.h"
// used: viewmatrix_t
#include "../sdk/datatypes/matrix.h"
#include "../core/sdk.h"
#include "../sdk/entity.h"
#include "../core/silentvmt/ShadowVMT.h"
#include "../core/silentvmt/InlineHook.h"
namespace VTABLE
{
	namespace D3D
	{
		enum
		{
			PRESENT = 8U,
			RESIZEBUFFERS = 13U,
			RESIZEBUFFERS_CSTYLE = 39U,
		};
	}

	namespace DXGI
	{
		enum
		{
			CREATESWAPCHAIN = 10U,
		};
	}

	namespace CLIENT
	{
		enum
		{
			CREATEMOVE = 5U,
			CAMERA = 7U,
			PREDICTION = 15U,
			MOUSEINPUTENABLED = 14U,
			FRAMESTAGENOTIFY = 33U,

		};
	}

	namespace INVENTORY
	{
		enum {

			EQUIPITEMLOADOUT = 54U,

		};
	}
}
class CNetInputMessage;
class CRenderGameSystem;
class IViewRender;
class CCSGOInput;
class CViewSetup;
class IGameEvent;
class C_BaseModelEntity;
class material_data_t;
class CGCClientSystem;
class CGCClientSharedObjectTypeCache;
class CCSGOInputHistoryEntryPB;
class CCSInputMessage;
namespace sigs {
	extern CSigScan GetHitboxSet;
	extern CSigScan HitboxToWorldTransforms;
	extern CSigScan ComputeHitboxSurroundingBox;
	extern CSigScan GetBoneName;
}  // namespace signatures

namespace H
{
	bool Setup();
	void Destroy();

	/* @section: handlers */
	// d3d11 & wndproc
	HRESULT WINAPI Present(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags);
	HRESULT CS_FASTCALL ResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags);
	HRESULT WINAPI CreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
	long CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// game's functions
	ViewMatrix_t* CS_FASTCALL GetMatrixForView(CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels);
	bool CS_FASTCALL CreateMove(CCSGOInput* pInput, int nSlot, bool nUnk, std::byte nUnk2);
	void CS_FASTCALL PredictionSimulation(CCSGOInput* pInput, int nSlot, CUserCmd* cmd);
	void CS_FASTCALL AllowCameraAngleChange(CCSGOInput* pCSGOInput, int a2);

	double CS_FASTCALL WeaponAcurracySpreadServerSide(void* a1);
	bool CS_FASTCALL FireEventClientSide(void* rcx, IGameEvent* event, bool bServerOnly);
	double CS_FASTCALL WeaponAcurracySpreadClientSide(void* a1);
	bool CS_FASTCALL EquipItemInLoadout(void* rcx, int iTeam, int iSlot, uint64_t iItemID);
	bool CS_FASTCALL MouseInputEnabled(void* pThisptr);
	void* CS_FASTCALL OnAddEntity(void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle);
	void* CS_FASTCALL OnRemoveEntity(void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle);
	void CS_FASTCALL FrameStageNotify(void* rcx, int nFrameStage);
	__int64* CS_FASTCALL LevelInit(void* pClientModeShared, const char* szNewMap);
	__int64 CS_FASTCALL LevelShutdown(void* pClientModeShared);
	void CS_FASTCALL OverrideView(void* pClientModeCSNormal, CViewSetup* pSetup);
	void CS_FASTCALL DrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, material_data_t* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);
	void CS_FASTCALL HandleGameEvents(void* rcx, IGameEvent* const event);
	void CS_FASTCALL SetModel(void* rcx, const char* model_name);
	int64_t CS_FASTCALL	SendNetInputMessage(CNetInputMessage* a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6);
	void CS_FASTCALL CameraInput(void* Input, int a1);
	float CS_FASTCALL SetViewModelFOV();
	float CS_FASTCALL GetRenderFov(uintptr_t rcx);
	void* CS_FASTCALL InputParser(CCSInputMessage* a1, CCSGOInputHistoryEntryPB* a2, char a3, void* a4, void* a5, void* a6);

	inline ShadowVMT SilenthkPresent{ };
	inline ShadowVMT Silentdxgi{ };
	inline ShadowVMT SilentInput{ };
	inline ShadowVMT SilentEntitySystem{ };
	inline ShadowVMT SilentInvetoryManager{ };
	inline CCSInventoryManager* (*fnGetInventoryManager)();
	inline CGCClientSystem* (*fnGetClientSystem)();
	inline void* (CS_FASTCALL* fnSetDynamicAttributeValueUint)(void*, void*,
		void*);
	inline CEconItem* (*fnCreateSharedObjectSubclassEconItem)();
	inline void(CS_FASTCALL* SetMeshGroupMask)(void*, uint64_t);

	inline int(CS_FASTCALL* fnFindMaterialIndex)(void*, void*, int, bool);

	inline CBaseHookObject<decltype(&GetMatrixForView)> hkGetMatrixForView = {};
	inline CBaseHookObject<decltype(&SetViewModelFOV)> hkSetViewModelFOV = {};
	inline CBaseHookObject<decltype(&GetRenderFov)> hkFOVObject = {};
	inline CBaseHookObject<decltype(&HandleGameEvents)> hkGameEvents = {};
	inline CBaseHookObject<decltype(&LevelInit)> hkLevelInit = {};
	inline CBaseHookObject<decltype(&DrawObject)> hkDrawObject = {};
	inline CBaseHookObject<decltype(&LevelShutdown)> hkLevelShutdown = {};
	inline CBaseHookObject<decltype(&WeaponAcurracySpreadClientSide)> hkWeapoSpreadClientSide = {};
	inline CBaseHookObject<decltype(&WeaponAcurracySpreadServerSide)> hkWeapoSpreadServerSide = {};
	inline CBaseHookObject<decltype(&FireEventClientSide)> hkPreFireEvent = {};
	inline CBaseHookObject<decltype(&FrameStageNotify)> hkFrameStageNotify = {};
	inline CBaseHookObject<decltype(&SetModel)> hkSetModel = {};
	inline CBaseHookObject<decltype(&EquipItemInLoadout)> hkEquipItemInLoadout = {};
	inline CBaseHookObject<decltype(&PredictionSimulation)> hkPredictionSimulation = {};
	inline CBaseHookObject<decltype(&SendNetInputMessage)> hkSendInputMessage = {};
	inline CBaseHookObject<decltype(&AllowCameraAngleChange)> hkAllowCameraChange = {};
	inline CBaseHookObject<decltype(&CameraInput)> hkCameraInput = {};
	inline CBaseHookObject<decltype(&OverrideView)> hkOverrideView = {};
	inline CBaseHookObject<decltype(&InputParser)> hkInputParser = {};


}
