#include "hooks.h"

// used: variables
#include "variables.h"

// used: game's sdk
#include "../sdk/interfaces/iswapchaindx11.h"
#include "../sdk/interfaces/iviewrender.h"
#include "../sdk/interfaces/cgameentitysystem.h"
#include "../sdk/interfaces/ccsgoinput.h"
#include "../sdk/interfaces/iinputsystem.h"
#include "../sdk/interfaces/iengineclient.h"
#include "../sdk/interfaces/inetworkclientservice.h"
#include "../sdk/interfaces/iglobalvars.h"
#include "../sdk/interfaces/imaterialsystem.h"
#include "../core/memory/cmodule.hpp"
#include "../features/visuals/overlay.h"
#include "../features/visuals/overlay.h"
#include "../features/legit/legit.h"
#include "../features/lagcomp/lagcomp.h"
#include "../features/skins/ccsinventorymanager.hpp"
#include "../cstrike/features/skins/skin_changer.hpp"
#include "../cstrike/features/antiaim/antiaim.hpp"

// used: viewsetup
#include "../sdk/datatypes/viewsetup.h"

// used: entity
#include "../sdk/entity.h"

// used: get virtual function, find pattern, ...
#include "../utilities/memory.h"
// used: inputsystem
#include "../utilities/inputsystem.h"
// used: draw
#include "../utilities/draw.h"

// used: features callbacks
#include "../features.h"

// used: game's interfaces
#include "interfaces.h"
#include "sdk.h"

// used: menu
#include "menu.h"
#define STB_OMIT_TESTS
#include "stb.hh"
#include "../sdk/interfaces/events.h"
#include "../cstrike/features/legit/legit.h"
#include "spoofcall/invoker.h"
#include "convars.h"
#include "../sdk/interfaces/ienginecvar.h"
#include "../cstrike/features/rage/rage.h"
#include "spoofcall/virtualization/VirtualizerSDK64.h"
#include <dxgi.h>
#include <d3d11.h>
#define SDK_SIG(sig) stb::simple_conversion::build<stb::fixed_string{sig}>::value
IDXGIDevice* pDXGIDevice = NULL;
IDXGIAdapter* pDXGIAdapter = NULL;
IDXGIFactory* pIDXGIFactory = NULL;
#define MEMORY_VARIABLE(var) var, "H::" #var

namespace sigs {
	
	CSigScan GetHitboxSet("C_BaseEntity::GetHitboxSet", "client.dll",
		{
			{SDK_SIG("E8 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 44 8D 48 07"), [](CPointer& ptr) { ptr.Absolute(1, 0); }},
			{SDK_SIG("41 8B D6 E8 ? ? ? ? 4C 8B F8"), [](CPointer& ptr) { ptr.Absolute(4, 0); }},
		});
	CSigScan GetBoneName("CModel::GetBoneName", "client.dll",
		{
			{SDK_SIG("48 8B CE E8 ? ? ? ? 48 8B 0F"), [](CPointer& ptr) { ptr.Offset(0x4); }},
		});
	CSigScan HitboxToWorldTransforms("C_BaseEntity::HitboxToWorldTransforms", "client.dll",
		{
			{SDK_SIG("E8 ? ? ? ? 4C 8B A3"), [](CPointer& ptr) { ptr.Absolute(1, 0); }},
		});


	CSigScan ComputeHitboxSurroundingBox("C_BaseEntity::ComputeHitboxSurroundingBox", "client.dll",
		{
			{SDK_SIG("E9 ? ? ? ? F6 43 5B FD"), [](CPointer& ptr) { ptr.Absolute(1, 0); }},
		});
} 
#define CS2_SDK_SIGs(sig) \
    stb::simple_conversion::build<stb::fixed_string{sig}>::value
#define COMPUTE_HITBOX_SURROUNDING_BOX CS2_SDK_SIGs("E9 ? ? ? ? F6 43 5B FD")

bool H::Setup()
{
	VIRTUALIZER_MUTATE_ONLY_START;
	if (MH_Initialize() != MH_OK)
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to initialize minhook");

		return false;
	}
	L_PRINT(LOG_INFO) << CS_XOR("Initializing ShadowVMT & SilentInlineHoooking");

	SilenthkPresent.Setup(I::SwapChain->pDXGISwapChain);
	SilenthkPresent.HookIndex((int)VTABLE::D3D::PRESENT, Present);
	SilenthkPresent.HookIndex((int)VTABLE::D3D::RESIZEBUFFERS, ResizeBuffers);

	I::Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));
	pDXGIDevice->GetAdapter(&pDXGIAdapter);
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));

	Silentdxgi.Setup(pIDXGIFactory);
	Silentdxgi.HookIndex((int)VTABLE::DXGI::CREATESWAPCHAIN, CreateSwapChain);


	if (pDXGIDevice) {
		pDXGIDevice->Release();
		pDXGIDevice = nullptr;
	}

	if (pDXGIAdapter) {
		pDXGIAdapter->Release();
		pDXGIAdapter = nullptr;
	}

	if (pIDXGIFactory) {
		pIDXGIFactory->Release();
		pIDXGIFactory = nullptr;
	}

	//	fnGetClientSystem = reinterpret_cast<CGCClientSystem*>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8B 4F 10 8B 1D ? ? ? ?")), 0x1, 0x0));
	//	L_PRINT(LOG_INFO) << CS_XOR("captured fnGetClientSystem \"") << CS_XOR("\" interface at address: ") << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<std::uintptr_t>(fnGetClientSystem);

	MEM::FindPatterns(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8B 4F 10 8B 1D ? ? ? ?")).ToAbsolute(1, 0).Get(MEMORY_VARIABLE(fnGetClientSystem));
	MEM::FindPatterns(CLIENT_DLL, CS_XOR("48 83 EC 28 B9 ? ? ? ? E8 ? ? ? ? 48 85 C0 74 3A 48 8D 0D ? ? ? ? C7 40 ? ? ? ? ?")).Get(MEMORY_VARIABLE(fnCreateSharedObjectSubclassEconItem));
	MEM::FindPatterns(CLIENT_DLL, CS_XOR("E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC 49 8B C0 48")).ToAbsolute(1, 0).Get(MEMORY_VARIABLE(fnSetDynamicAttributeValueUint));
	MEM::FindPatterns(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 63 BB ? ? ? ? 48 8D 68 28 83 FF FF")).ToAbsolute(1, 0).Get(MEMORY_VARIABLE(fnGetInventoryManager));
	MEM::FindPatterns(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8D 99 60")).Get(MEMORY_VARIABLE(SetMeshGroupMask));
	MEM::FindPatterns(CLIENT_DLL, CS_XOR("48 89 5C 24 10 48 89 6C 24 18 56 57 41 57 48 83 EC 20 83 79 10 00 49 8B F1 41 8B E8 4C 8B FA 48 8B D9 74 72 44 8B 49 14 48 8B 39 41 FF C9 45 8B D9 45 23 D8 41")).Get(MEMORY_VARIABLE(fnFindMaterialIndex));

	SilentEntitySystem.Setup(I::GameResourceService->pGameEntitySystem);
	SilentEntitySystem.HookIndex(14, OnAddEntity);
	SilentEntitySystem.HookIndex(15, OnRemoveEntity);

	SilentInput.Setup(I::Input);
	SilentInput.HookIndex((int)VTABLE::CLIENT::MOUSEINPUTENABLED, MouseInputEnabled);
	SilentInput.HookIndex((int)VTABLE::CLIENT::CREATEMOVE, CreateMove);

	spoof_call<void>(_fake_addr, &EntCache::CacheCurrentEntities);

	// @ida: class CViewRender->OnRenderStart call GetMatricesForView
	CS_ASSERT(hkGetMatrixForView.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 81 EC ? ? ? ? 49 8B C1")), reinterpret_cast<void*>(&GetMatrixForView)));

	CS_ASSERT(hkSetViewModelFOV.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 83 EC 30 33 C9 E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 48 8B 00 48 8B CB FF 90 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 8B CB")), reinterpret_cast<float*>(&SetViewModelFOV)));
	
	
	CS_ASSERT(hkFOVObject.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 81 EC 80 00 00 00 48 8B D9 E8 ?? ?? ?? ?? 48 85")), reinterpret_cast<float*>(&GetRenderFov)));

	CS_ASSERT(hkInputParser.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 4C 89 48 20 55 56 41 56 48 8D 68 B1 48 81 EC D0 00 00 00")), reinterpret_cast<void*>(&InputParser)));

	// client.dll; 40 55 53 41 55 41 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B 02
	CS_ASSERT(hkGameEvents.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 55 53 41 55 41 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B 02")), reinterpret_cast<void*>(&HandleGameEvents)));

	// I could just do VTABLE too idx: 15
	//CS_ASSERT(hkPredictionSimulation.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 4C 89 40 18 48 89 48 08 55 53 56 57 48")), reinterpret_cast<void*>(&PredictionSimulation)));

	// @ida: ClientModeShared -> #STR: "mapname", "transition", "game_newmap"
	CS_ASSERT(hkLevelInit.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 56 48 83 EC ? 48 8B 0D ? ? ? ? 48 8B F2")), reinterpret_cast<void*>(&LevelInit)));

	// @ida: ClientModeShared -> #STR: "map_shutdown"
	CS_ASSERT(hkLevelShutdown.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 48 8B 01 FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 4C 8B 01 48 83 C4 ? 49 FF 60 ? 48 83 C4 ? C3 CC CC CC 48 83 EC ? 4C 8B D9")), reinterpret_cast<void*>(&LevelShutdown)));

	CS_ASSERT(hkDrawObject.Create(MEM::FindPattern(SCENESYSTEM_DLL, CS_XOR("48 8B C4 48 89 50 ? 55 41 56")), reinterpret_cast<void*>(&DrawObject)));

	//CS_ASSERT(hkCameraInput.Create(MEM::GetVFunc(I::Input, VTABLE::CLIENT::CAMERA), reinterpret_cast<void*>(&CameraInput)));

	//CS_ASSERT(hkSendInputMessage.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 49 8B D9")), reinterpret_cast<void*>(&SendNetInputMessage)));

	CS_ASSERT(hkFrameStageNotify.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 10 56 48 83 EC 30 8B 05")), reinterpret_cast<void*>(&FrameStageNotify)));

	CS_ASSERT(hkSetModel.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 10 48 89 7C 24 20 55 48 8B EC 48 83 EC 50")), reinterpret_cast<void*>(&SetModel)));

	CS_ASSERT(hkEquipItemInLoadout.Create(MEM::GetVFunc(CCSInventoryManager::GetInstance(), 54u), reinterpret_cast<void*>(&EquipItemInLoadout)));

	CS_ASSERT(hkOverrideView.Create(MEM::GetVFunc(I::Input, 9u), reinterpret_cast<void*>(&OverrideView)));

	CS_ASSERT(hkAllowCameraChange.Create(MEM::GetVFunc(I::Input, 7u), reinterpret_cast<void*>(&AllowCameraAngleChange)));

	//CS_ASSERT(hkPreFireEvent.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 56 57 41 54 48 83 EC 30 48 8B F2")), reinterpret_cast<void*>(&FireEventClientSide)));

	F::RAGE::rage->BuildSeed();
	F::LAGCOMP::lagcomp->Initialize();

	//F::LAGCOMP::lagcomp->Initialize();

	VIRTUALIZER_MUTATE_ONLY_END;

	return true;
}


void H::Destroy()
{
	skin_changer::Shutdown();
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	MH_Uninitialize();

	SilenthkPresent.UnhookAll();
	Silentdxgi.UnhookAll();
	SilentInput.UnhookAll();
	SilentEntitySystem.UnhookAll();
}
// client.dll; 40 55 53 41 55 41 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B 02
// xref: "winpanel-basic-round-result-visible"

using namespace F::RAGE;

void* CS_FASTCALL H::InputParser(CCSInputMessage* Input, CCSGOInputHistoryEntryPB* InputHistoryEntry, char a3, void* a4, void* a5, void* a6)
{
	auto original = hkInputParser.GetOriginal();

	auto result = original(Input, InputHistoryEntry, a3, a4, a5, a6);

	if (!SDK::Cmd)
		return result;

	if (I::Engine->IsConnected() || I::Engine->IsInGame() && SDK::LocalPawn && SDK::LocalPawn->GetHealth() > 0 ) {
		
		if (rage->sub_tick_data.command == F::RAGE::impl::command_msg::silent) {
			// modify predicted sub tick viewangles to our rage bestpoint forcing silent w predicted cmds
			Input->m_view_angles = rage->sub_tick_data.best_point_vec;
			Input->m_view_angles.clamp();

			InputHistoryEntry->m_pViewCmd->m_angValue = rage->sub_tick_data.best_point;
			InputHistoryEntry->m_pViewCmd->m_angValue.Clamp();
			rage->sub_tick_data.response = impl::response_msg::validated_view_angles;

		}
		else 
			rage->sub_tick_data.response = impl::response_msg::empty;

		if (F::RAGE::rage->rage_data.rapid_fire) {
			if (rage->sub_tick_data.command == impl::command_msg::rapid_fire) {
				// override angles just to prevent
				Input->m_view_angles = rage->sub_tick_data.best_point_vec;
				Input->m_view_angles.clamp();
				InputHistoryEntry->m_pViewCmd->m_angValue = rage->sub_tick_data.best_point;
				InputHistoryEntry->m_pViewCmd->m_angValue.Clamp();

				// override tickcount
				InputHistoryEntry->m_nPlayerTickCount = 0;
				Input->m_player_tick_count = 0;

				// perform shooting
				SDK::Cmd->m_nButtons.m_nValue |= IN_ATTACK;
			}
			else
				SDK::Cmd->m_nButtons.m_nValue &= ~IN_ATTACK;
		}

		if (rage->sub_tick_data.command == impl::command_msg::teleport) {
			// overflow viewangles
			// send new msg to sv 
			// maybe change our shoot pos? idk 
		}
	}


	return result;
}
//40 53 48 83 EC 20 48 8B D9 E8 ?? ?? ?? ?? 48 C7 83 D0
void CS_FASTCALL H::CameraInput(void* Input, int a1) {
	//auto horiginal = hkCameraInput.GetOriginal();

	//if (!I::Engine->IsConnected() || !I::Engine->IsInGame() || !SDK::LocalPawn || !cheat->alive)
	//	return horiginal(Input, a1);

	//auto backup = *(Vector_t*)((uintptr_t)Input + 0x5390);
	//// store old camera angles

	//// call original
	//horiginal(Input, a1);

	//*(Vector_t*)((uintptr_t)Input + 0x5390) = backup;
}

float CS_FASTCALL H::GetRenderFov(uintptr_t rcx)
{
	const auto oFOV = hkFOVObject.GetOriginal();

	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
	{
		oFOV(rcx);
	}
	else if (!SDK::LocalController || !SDK::LocalController->IsPawnAlive()) // Checking if your spectating and alive
	{
		oFOV(rcx);
	}
	else
	{
		auto localPlayerController = SDK::LocalController;
		auto localPlayerPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(localPlayerController->GetPawnHandle());

		if (C_GET(bool, Vars.bFOV) && !localPlayerPawn->IsScoped())
		{
			return C_GET(float, Vars.fFOVAmount);
		}
		else
		{
			oFOV(rcx);
		}
	}
}

float CS_FASTCALL H::SetViewModelFOV()
{
	const auto oSetViewModelFOV = hkSetViewModelFOV.GetOriginal();

	if (C_GET(float, Vars.flSetViewModelFOV) == 0.f)
		return oSetViewModelFOV();

	return C_GET(float, Vars.flSetViewModelFOV);
}
// client.dll; 40 55 53 41 55 41 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B 02
// xref: "winpanel-basic-round-result-visible"
void CS_FASTCALL H::HandleGameEvents(void* rcx, IGameEvent* const ev)
{
	const auto original = hkGameEvents.GetOriginal();
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame() || !SDK::LocalPawn || !cheat->alive)
		return 	original(rcx, ev);

	switch (const char* event_name{ ev->GetName() }; FNV1A::Hash(event_name)) {
	case FNV1A::HashConst(CS_XOR("player_death")): {
		auto controller = SDK::LocalController;
		if (!controller)
			break;

		const auto event_controller = ev->get_player_controller("attacker");
		if (!event_controller)
			return;

		skin_changer::OnPreFireEvent(ev);

		F::LEGIT::legit->Events(ev, F::LEGIT::events::player_death);

	} break;
	case FNV1A::HashConst(CS_XOR("round_start")): {
		auto controller = SDK::LocalController;
		if (!controller)
			break;

		// fix skins not showing up on round start event
		Vars.full_update = true;

		// reset some shit related to legitbot
		F::LEGIT::legit->Events(ev, F::LEGIT::events::round_start);

	} break;
	default:
		break;
	}

	original(rcx, ev);
}
bool init = false;
HRESULT __stdcall H::Present(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags)
{
	auto hResult = SilenthkPresent.GetOg<decltype(&Present)>((int)VTABLE::D3D::PRESENT);

	if (!init && I::RenderTargetView == nullptr)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&I::Device)))
		{
			I::Device->GetImmediateContext(&I::DeviceContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			IPT::hWindow = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			I::Device->CreateRenderTargetView(pBackBuffer, NULL, &I::RenderTargetView);
			pBackBuffer->Release();
			D::InitImGui();
			init = true;
		}

		else
			return hResult(pSwapChain, uSyncInterval, uFlags);
	}

	D::Render();

	return hResult(pSwapChain, uSyncInterval, uFlags);
}
#include"../dependencies/imgui/imgui_impl_dx11.h"
#include"../dependencies/imgui/imgui_impl_win32.h"

HRESULT CS_FASTCALL H::ResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags)
{
	ImGui_ImplDX11_InvalidateDeviceObjects();

	auto hResult = SilenthkPresent.GetOg<decltype(&ResizeBuffers)>((int)VTABLE::D3D::RESIZEBUFFERS);
	if (SUCCEEDED(hResult))
		I::CreateRenderTarget(pSwapChain);

	return hResult(pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags);
}

HRESULT __stdcall H::CreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	auto hResult = Silentdxgi.GetOg<decltype(&CreateSwapChain)>((int)VTABLE::DXGI::CREATESWAPCHAIN);
	I::DestroyRenderTarget();
	L_PRINT(LOG_INFO) << CS_XOR("render target view has been destroyed");

	return hResult(pFactory, pDevice, pDesc, ppSwapChain);

}

long H::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (D::OnWndProc(hWnd, uMsg, wParam, lParam))
		return 1L;

	return ::CallWindowProcW(IPT::pOldWndProc, hWnd, uMsg, wParam, lParam);
}

void* CS_FASTCALL H::OnAddEntity(void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle) {
	spoof_call<void>(_fake_addr, EntCache::OnAddEntity, pInstance, hHandle);

	return SilentEntitySystem.GetOg<decltype(&OnAddEntity)>(14)(rcx, pInstance, hHandle);
}

void* CS_FASTCALL H::OnRemoveEntity(void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle) {
	spoof_call<void>(_fake_addr, EntCache::OnRemoveEntity, pInstance, hHandle);

	return SilentEntitySystem.GetOg<decltype(&OnRemoveEntity)>(15)(rcx, pInstance, hHandle);
}

ViewMatrix_t* CS_FASTCALL H::GetMatrixForView(CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels)
{    // Call the original function
	const auto oGetMatrixForView = hkGetMatrixForView.GetOriginal();
	ViewMatrix_t* matResult = oGetMatrixForView(pRenderGameSystem, pViewRender, pOutWorldToView, pOutViewToProjection, pOutWorldToProjection, pOutWorldToPixels);	// get view matrix
	SDK::ViewMatrix = *pOutWorldToProjection;
	// get camera position
	// @note: ida @GetMatrixForView(global_pointer, pRenderGameSystem + 16, ...)
	SDK::CameraPosition = pViewRender->vecOrigin;

	// keep bounding box updated
	if (I::Engine->IsConnected() && I::Engine->IsInGame())
		spoof_call<void>(_fake_addr, &F::VISUALS::OVERLAY::CalculateBoundingBoxes);


	return matResult;
}
// SendNetInputMessage - 48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 49 8B D9

int64_t CS_FASTCALL H::SendNetInputMessage(CNetInputMessage* a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6)
{
	const auto bres = hkSendInputMessage.GetOriginal();

	return bres(a1, a2, a3, a4, a5, a6);
}

#include "..\cstrike\sdk\datatypes\usercmd.h"
#include "../features/misc.h"
#include "../features/misc/movement.h"

void H::AllowCameraAngleChange(CCSGOInput* pCSGOInput, int a2)
{
	const auto Original = hkAllowCameraChange.GetOriginal();

	if (!I::Engine->IsInGame() || !I::Engine->IsConnected() || pCSGOInput == nullptr)
		return Original(pCSGOInput, a2);

	CUserCmd* pCmd = pCSGOInput->GetUserCmd();
	if (pCmd == nullptr)
		return Original(pCSGOInput, a2);

	QAngle_t angOriginalAngle = pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue;
	pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue = F::ANTIAIM::angStoredViewBackup;
	Original(pCSGOInput, a2);
	pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue = angOriginalAngle;
}

bool CS_FASTCALL H::CreateMove(CCSGOInput* pInput, int nSlot, bool nUnk, std::byte nUnk2)
{
	const bool bResult = SilentInput.GetOg<decltype(&CreateMove)>((int)VTABLE::CLIENT::CREATEMOVE)(pInput, nSlot, nUnk, nUnk2);

	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return bResult;

	CUserCmd* pCmd = SDK::Cmd = pInput->GetUserCmd();
	if (pCmd == nullptr)
		return bResult;

	F::ANTIAIM::angStoredViewBackup = pCmd->m_csgoUserCmd.m_pBaseCmd->m_pViewangles->m_angValue;

	SDK::LocalController = CCSPlayerController::GetLocalPlayerController();
	if (SDK::LocalController == nullptr)
		return bResult;

	SDK::LocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(SDK::LocalController->GetPawnHandle());
	if (SDK::LocalPawn == nullptr)
		return bResult;

	F::ANTIAIM::RunAA(pCmd);

	cheat->alive = SDK::LocalPawn->GetHealth() > 0 && SDK::LocalPawn->GetLifeState() != ELifeState::LIFE_DEAD;
	cheat->onground = (SDK::LocalPawn->GetFlags() & FL_ONGROUND);
	cheat->canShot = SDK::LocalPawn->CanShoot(I::GlobalVars->nTickCount);

	if (cheat->alive) {
		auto network = I::NetworkClientService->GetNetworkClient();
		if (network && Vars.full_update) {
			network->Update();
			Vars.full_update = false;
		}

		// process legit bot 
		if (C_GET(bool, Vars.legit_enable)) {
			F::LEGIT::legit->SetupAdaptiveWeapon(SDK::LocalPawn);
			F::LEGIT::legit->Run(pCmd);
		}

		F::LAGCOMP::lagcomp->Start(pCmd);

	
		// we should run engine pred here for movement features etc
		// in order to predict ( velocity, flags, origin )
		// setup menu adaptive weapon with rage data
		if (C_GET(bool, Vars.rage_enable)) {
			F::RAGE::rage->SetupAdaptiveWeapon(SDK::LocalPawn);
			F::RAGE::rage->Run(SDK::LocalPawn, pInput, pCmd);
		}

		F::MISC::MOVEMENT::ProcessMovement(pCmd, SDK::LocalController, SDK::LocalPawn);
	}

	return bResult;
}
#include "../features/misc/movement.h"
void CS_FASTCALL H::PredictionSimulation(CCSGOInput* pInput, int nSlot, CUserCmd* pCmd)
{
	static auto bResult = SilentInput.GetOg<decltype(&PredictionSimulation)>((int)VTABLE::CLIENT::PREDICTION);

	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return bResult(pInput, nSlot, pCmd);

	CUserCmd* commandnr = pInput->GetUserCmd();
	if (commandnr == nullptr)
		return  bResult(pInput, nSlot, pCmd);

	auto predicted_local_controller = CCSPlayerController::GetLocalPlayerController();
	if (predicted_local_controller == nullptr)
		return bResult(pInput, nSlot, pCmd);

	auto pred = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(predicted_local_controller->m_hPredictedPawn());
	if (pred == nullptr)
		return bResult(pInput, nSlot, pCmd);

	// run auto stop w predicted cmd 
//	F::RAGE::rage->AutomaticStop(pred, pred->ActiveWeapon(), pCmd);

	// fix our command number members for predicted values (move correction etc)

	return bResult(pInput, nSlot, pCmd);
}


double CS_FASTCALL H::WeaponAcurracySpreadClientSide(void* a1) {
	static auto fnWeaponAcurracySpreadClientSide = hkWeapoSpreadClientSide.GetOriginal();

	if (C_GET(bool, Vars.remove_weapon_accuracy_spread))
		return 0.0;

	return fnWeaponAcurracySpreadClientSide(a1);
}

double CS_FASTCALL H::WeaponAcurracySpreadServerSide(void* a1) {
	static auto fnWeaponAcurracySpreadServerSide = hkWeapoSpreadServerSide.GetOriginal();

	if (C_GET(bool, Vars.remove_weapon_accuracy_spread))
		return 0.0;

	return fnWeaponAcurracySpreadServerSide(a1);
}


bool CS_FASTCALL H::FireEventClientSide(void* rcx, IGameEvent* event, bool bServerOnly)
{
	const auto oPreFire = hkPreFireEvent.GetOriginal();

	return oPreFire(rcx, event, bServerOnly);
}

bool CS_FASTCALL H::MouseInputEnabled(void* pThisptr)
{
	return MENU::bMainWindowOpened ? false : SilentInput.GetOg<decltype(&MouseInputEnabled)>((int)VTABLE::CLIENT::MOUSEINPUTENABLED)(pThisptr);
}
void CS_FASTCALL H::SetModel(void* rcx, const char* model) {
	const auto oSetModel = hkSetModel.GetOriginal();
	skin_changer::OnSetModel((C_BaseModelEntity*)rcx, model);
	return oSetModel(rcx, model);
}

bool CS_FASTCALL H::EquipItemInLoadout(void* rcx, int iTeam, int iSlot,
	uint64_t iItemID) {
	const auto oEquipItemInLoadout = hkEquipItemInLoadout.GetOriginal();
	skin_changer::OnEquipItemInLoadout(iTeam, iSlot, iItemID);
	return oEquipItemInLoadout(rcx, iTeam, iSlot, iItemID);
}

void CS_FASTCALL H::FrameStageNotify(void* rcx, int nFrameStage)
{
	const auto oFrameStageNotify = hkFrameStageNotify.GetOriginal();

	if (I::Engine->IsConnected() && I::Engine->IsInGame())
		skin_changer::OnFrameStageNotify(nFrameStage);


	return oFrameStageNotify(rcx, nFrameStage);
}

__int64* CS_FASTCALL H::LevelInit(void* pClientModeShared, const char* szNewMap)
{
	const auto oLevelInit = hkLevelInit.GetOriginal();
	// if global variables are not captured during I::Setup or we join a new game, recapture it
	if (I::GlobalVars == nullptr)
		I::GlobalVars = *reinterpret_cast<IGlobalVars**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 0D ? ? ? ? 48 89 41")), 0x3, 0x7));

	return oLevelInit(pClientModeShared, szNewMap);
}
//0x8D4C00000D1C8310
// client.dll 7FFB83FD0000
__int64 CS_FASTCALL H::LevelShutdown(void* pClientModeShared)
{
	const auto oLevelShutdown = hkLevelShutdown.GetOriginal();
	// reset global variables since it got discarded by the game
	I::GlobalVars = nullptr;

	return oLevelShutdown(pClientModeShared);
}

void CS_FASTCALL H::OverrideView(void* pClientModeCSNormal, CViewSetup* pSetup)
{
	const auto oOverrideView = hkOverrideView.GetOriginal();
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return hkOverrideView.GetOriginal()(pClientModeCSNormal, pSetup);

	if (!SDK::LocalController->IsPawnAlive() || !SDK::LocalController)
		return hkOverrideView.GetOriginal()(pClientModeCSNormal, pSetup);

	static auto progress = 0.f;
	if (C_GET(bool, Vars.bThirdperson))
	{
		auto bezier = [](const float t)
		{
			return t * t * (3.0f - 2.0f * t);
		};

		progress = MATH::clamp(progress + I::GlobalVars->flFrameTime * 6.f, 40.f / C_GET(float, Vars.flThirdperson), 1.f);

		CONVAR::cam_idealdist->value.fl = C_GET(float, Vars.flThirdperson) * (C_GET(bool, Vars.bThirdpersonNoInterp) ? 1.f : bezier(progress));
		CONVAR::cam_collision->value.i1 = true;
		CONVAR::cam_snapto->value.i1 = true;
		CONVAR::c_thirdpersonshoulder->value.i1 = true;
		CONVAR::c_thirdpersonshoulderaimdist->value.fl = 0.f;
		CONVAR::c_thirdpersonshoulderdist->value.fl = 0.f;
		CONVAR::c_thirdpersonshoulderheight->value.fl = 0.f;
		CONVAR::c_thirdpersonshoulderoffset->value.fl = 0.f;

		I::Input->bInThirdPerson = true;
	}
	else
	{
		progress = C_GET(bool, Vars.bThirdperson) ? 1.f : 0.f;
		I::Input->bInThirdPerson = false;
	}

	oOverrideView(pClientModeCSNormal, pSetup);
}
//8D4C7FFB91198310
void CS_FASTCALL H::DrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, material_data_t* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	const auto oDrawObject = hkDrawObject.GetOriginal();
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

	if (SDK::LocalController == nullptr || SDK::LocalPawn == nullptr)
		return oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

	if (!F::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2))
		oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}

