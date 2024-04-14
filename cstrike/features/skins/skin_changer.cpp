#include <vector>

#include "skin_changer.hpp"

#include "../cstrike/sdk/interfaces/iengineclient.h"
#include "../../core/config.h"
#include "../../core/variables.h"
#include "../../sdk/datatypes/usercmd.h"
#include "../../core/sdk.h"
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/iglobalvars.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../cstrike/sdk/interfaces/iengineclient.h"
#include "../../sdk/datatypes/qangle.h"
#include "../../sdk/datatypes/vector.h"
#include "../cstrike/sdk/interfaces/inetworkclientservice.h"
#include "../cstrike/sdk/interfaces/ccsgoinput.h"
#include "../misc/movement.h"
#include "../cstrike/sdk/interfaces/ccsgoinput.h"
#include "../cstrike/sdk/interfaces/ienginecvar.h"
#include "../lagcomp/lagcomp.h"
#include "../cstrike/sdk/interfaces/events.h"
#include "../penetration/penetration.h"
#include "../cstrike/sdk/interfaces/itrace.h"
#include "../cstrike/core/spoofcall/syscall.h"
#include <iostream>
#include <memoryapi.h>
#include <mutex>
#include <array>
#include "../cstrike/sdk/interfaces/iengineclient.h"
#include "../../core/spoofcall/virtualization/VirtualizerSDK64.h"
#include "../../utilities/inputsystem.h"
#include "ccsinventorymanager.hpp"
#include "ccsplayerinventory.hpp"
#include "../cstrike/core/hooks.h"
static std::vector<uint64_t> g_vecAddedItemsIDs;

static int glove_frame = 0;
struct GloveInfo {
    int itemId;
    uint64_t itemHighId;
    uint64_t itemLowId;
    int itemDefId;
};
static GloveInfo  addedGloves;

// Define a struct to hold glove information


struct material_info
{
    skin_changer::material_record* p_mat_records;
    uint32_t			 ui32_count;
};

void invalidate_glove_material(C_BaseViewModel* viewmodel)
{
    material_info* p_mat_info = reinterpret_cast<material_info*>(reinterpret_cast<uint8_t*>(viewmodel) + 0xf80);

    for (uint32_t i = 0; i < p_mat_info->ui32_count; i++)
    {
        if (p_mat_info->p_mat_records[i].identifer == skin_changer::material_magic_number__gloves)
        {
            p_mat_info->p_mat_records[i].ui32_type_index = 0xffffffff;
            break;
        }
    }
}
void skin_changer::OnGlove(CCSPlayerInventory* pInventory, C_CSPlayerPawn* pLocalPawn, C_BaseViewModel* pViewModel, C_EconItemView* GlovesItem, CEconItemDefinition* GlovesDefinition) {

    if (!pLocalPawn) return;

    if (pLocalPawn->GetHealth() <= 0)
        return;

    if (!pViewModel)
        return;

    if (!GlovesItem)
        return;

    if (!GlovesDefinition) return;

    if (addedGloves.itemId == 0)
        return;


    if (glove_frame)
    {
        invalidate_glove_material(pViewModel);

        GlovesItem->m_bInitialized() = true;
        pLocalPawn->m_bNeedToReApplyGloves() = true;

        glove_frame--;
    }

    if (GlovesItem->m_iItemID() != addedGloves.itemId) {

        glove_frame = 2;

        GlovesItem->m_bDisallowSOC() = false;

        GlovesItem->m_iItemID() = addedGloves.itemId;
        GlovesItem->m_iItemIDHigh() = addedGloves.itemHighId;
        GlovesItem->m_iItemIDLow() = addedGloves.itemLowId;
        GlovesItem->m_iAccountID() = uint32_t(pInventory->GetOwner().m_id);
        GlovesItem->m_iItemDefinitionIndex() = addedGloves.itemDefId;
        GlovesItem->m_bDisallowSOC() = false;

        pViewModel->GetGameSceneNode()->SetMeshGroupMask(1);

    }

}

void skin_changer::OnFrameStageNotify(int frameStage) {
    if (frameStage != 6) return;


    CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
    if (!pInventory) return;

    CGameEntitySystem* pEntitySystem = I::GameResourceService->pGameEntitySystem;
    if (!pEntitySystem) return;

    const uint64_t steamID = pInventory->GetOwner().m_id;

    CCSPlayerController* pLocalPlayerController = CCSPlayerController::GetLocalPlayerController();
    if (!pLocalPlayerController) return;

    C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalPlayerController->GetPawnHandle());
    if (!pLocalPawn) return;

    if (pLocalPawn->GetHealth() <= 0)
        return;

    CCSPlayer_ViewModelServices* pViewModelServices = pLocalPawn->GetViewModelServices();
    if (!pViewModelServices) return;
   
    C_CSGOViewModel* pViewModel = I::GameResourceService->pGameEntitySystem->Get<C_CSGOViewModel>(pViewModelServices->m_hViewModel());
    if (!pViewModel)
        return;
    C_EconItemView* pGloves = &pLocalPawn->m_EconGloves();
    CEconItemDefinition* pGlovesDefinition = pGloves->GetStaticData();

    skin_changer::OnGlove(pInventory, pLocalPawn, pViewModel, pGloves, pGlovesDefinition);


    int highestIndex = pEntitySystem->GetHighestEntityIndex();
    for (int i = 64 + 1; i <= highestIndex; ++i) {
        C_BaseEntity* pEntity = pEntitySystem->Get(i);
        if (!pEntity || !pEntity->IsWeapon()) continue;

        C_CSWeaponBase* pWeapon = reinterpret_cast<C_CSWeaponBase*>(pEntity);
        if (pWeapon->GetOriginalOwnerXuid() != steamID) continue;

        CAttributeManager* pAttributeContainer = &pWeapon->m_AttributeManager();
        if (!pAttributeContainer) continue;

        C_EconItemView* pWeaponItemView = &pAttributeContainer->m_Item();
        if (!pWeaponItemView) continue;

        CEconItemDefinition* pWeaponDefinition =
            pWeaponItemView->GetStaticData();
        if (!pWeaponDefinition) continue;

        CGameSceneNode* pWeaponSceneNode = pWeapon->GetGameSceneNode();
        if (!pWeaponSceneNode) continue;

        // No idea how to check this faster with the new loadout system.
        C_EconItemView* pWeaponInLoadoutItemView = nullptr;

        if (pWeaponDefinition->IsWeapon()) {
            for (int i = 0; i <= 56; ++i) {
                C_EconItemView* pItemView = pInventory->GetItemInLoadout(
                    pWeapon->m_iOriginalTeamNumber(), i);
                if (!pItemView) continue;

                if (pItemView->m_iItemDefinitionIndex() ==
                    pWeaponDefinition->m_nDefIndex) {
                    pWeaponInLoadoutItemView = pItemView;
                    break;
                }
            }
        }
        else {
            pWeaponInLoadoutItemView = pInventory->GetItemInLoadout(
                pWeapon->m_iOriginalTeamNumber(),
                pWeaponDefinition->GetLoadoutSlot());
        }

        if (!pWeaponInLoadoutItemView)
            continue;

        // Check if skin is added by us.
        auto it = std::find(g_vecAddedItemsIDs.cbegin(), g_vecAddedItemsIDs.cend(), pWeaponInLoadoutItemView->m_iItemID());
        if (it == g_vecAddedItemsIDs.cend()) continue;

        CEconItemDefinition* pWeaponInLoadoutDefinition = pWeaponInLoadoutItemView->GetStaticData();
        if (!pWeaponInLoadoutDefinition) continue;

        // Example: Will not equip FiveSeven skin on CZ. Not applies for knives.
        const bool isKnife = pWeaponInLoadoutDefinition->IsKnife(false, pWeaponInLoadoutDefinition->m_pszItemBaseName);
        L_PRINT(LOG_INFO) << "applying skin on:" << pWeaponInLoadoutDefinition->m_pszItemBaseName;
        pWeaponItemView->m_bDisallowSOC() = false;
        pWeaponInLoadoutItemView->m_bDisallowSOC() = false;

        pWeaponItemView->m_iItemID() = pWeaponInLoadoutItemView->m_iItemID();
        pWeaponItemView->m_iItemIDHigh() = pWeaponInLoadoutItemView->m_iItemIDHigh();
        pWeaponItemView->m_iItemIDLow() = pWeaponInLoadoutItemView->m_iItemIDLow();
        pWeaponItemView->m_iAccountID() = uint32_t(pInventory->GetOwner().m_id);
        pWeaponItemView->m_iItemDefinitionIndex() = pWeaponInLoadoutDefinition->m_nDefIndex;

        // pWeaponItemView->m_bIsStoreItem() = true;
       //pWeaponItemView->m_bIsTradeItem() = true;


        // Displays nametag and stattrak on the gun.
        // Found by: https://www.unknowncheats.me/forum/members/2377851.html
     /*   if (!pWeapon->m_bUIWeapon()) {
            pWeapon->AddStattrakEntity();
            pWeapon->AddNametagEntity();
        }*/

        CBaseHandle hWeapon = pWeapon->GetRefEHandle();
        if (isKnife) {
            if (pViewModel && (pViewModel->m_hWeapon().GetEntryIndex() == hWeapon.GetEntryIndex() || pViewModel->m_hWeapon() == hWeapon)) {
                pWeaponItemView->m_iItemDefinitionIndex() = pWeaponInLoadoutDefinition->m_nDefIndex;

                const char* knifeModel = pWeaponInLoadoutDefinition->GetModelName();
                     
                CGameSceneNode* pViewModelSceneNode = pViewModel->GetGameSceneNode();
                if (pViewModelSceneNode) {
                    pWeaponSceneNode->SetMeshGroupMask(2);
                    pViewModelSceneNode->SetMeshGroupMask(2);
                }
                pWeapon->SetModel(knifeModel);
                pViewModel->SetModel(knifeModel);
                pViewModel->pAnimationGraphInstance->pAnimGraphNetworkedVariables = nullptr;
            }
        }
        else {
            // Use legacy weapon models only for skins that require them.
            // Probably need to cache this if you really care that much about
            // performance

            const char* model = pWeaponInLoadoutDefinition->GetModelName();
            pWeapon->SetModel(model);
            if (pViewModel && pViewModel->m_hWeapon() == hWeapon)
            {
                pViewModel->SetModel(model);
            }

            pWeaponSceneNode->SetMeshGroupMask(2);
            if (pViewModel && pViewModel->m_hWeapon().GetEntryIndex() == hWeapon.GetEntryIndex()) {
                CGameSceneNode* pViewModelSceneNode = pViewModel->GetGameSceneNode();

                pViewModelSceneNode->SetMeshGroupMask(2);
            }
        }
    }
}
#include "../cstrike/sdk/interfaces/ccsgoinput.h"

void skin_changer::OnEquipItemInLoadout(int team, int slot, uint64_t itemID) {
    auto it =
        std::find(g_vecAddedItemsIDs.begin(), g_vecAddedItemsIDs.end(), itemID);
    if (it == g_vecAddedItemsIDs.end()) return;

    CCSInventoryManager* pInventoryManager = CCSInventoryManager::GetInstance();
    if (!pInventoryManager) return;

    CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
    if (!pInventory) return;

    C_EconItemView* pItemViewToEquip = pInventory->GetItemViewForItem(*it);
    if (!pItemViewToEquip) return;

    C_EconItemView* pItemInLoadout = pInventory->GetItemInLoadout(team, slot);
    if (!pItemInLoadout) return;

    CEconItemDefinition* pItemInLoadoutStaticData = pItemInLoadout->GetStaticData();
    if (!pItemInLoadoutStaticData)
        return;

    // Equip default item. If you would have bought Deagle and you previously
    // had R8 equipped it will now give you a Deagle.
    const uint64_t defaultItemID = (std::uint64_t(0xF) << 60) | pItemViewToEquip->m_iItemDefinitionIndex();
    pInventoryManager->EquipItemInLoadout(team, slot, defaultItemID);

    CEconItem* pItemInLoadoutSOCData = pItemInLoadout->GetSOCData(nullptr);
    if (!pItemInLoadoutSOCData)
        return;

    CEconItemDefinition* toequipdata = pItemViewToEquip->GetStaticData();
    if (!toequipdata)
        return;

    if (toequipdata->IsWeapon() && !toequipdata->IsKnife(false, pItemInLoadoutStaticData->m_pszItemTypeName) && !toequipdata->IsGlove(false, pItemInLoadoutStaticData->m_pszItemTypeName)) {

        pInventory->SOUpdated(pInventory->GetOwner(), (CSharedObject*)pItemInLoadoutSOCData, eSOCacheEvent_Incremental);
        return;
    }
    else  if (toequipdata->IsGlove(false, pItemInLoadoutStaticData->m_pszItemTypeName)) {
        const uint64_t steamID = pInventory->GetOwner().m_id;

        CCSPlayerController* pLocalPlayerController = CCSPlayerController::GetLocalPlayerController();
        if (!pLocalPlayerController) return;

        C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalPlayerController->GetPawnHandle());
        if (!pLocalPawn) return;

        if (pLocalPawn->GetHealth() <= 0)
            return;

        CCSPlayer_ViewModelServices* pViewModelServices = pLocalPawn->GetViewModelServices();
        if (!pViewModelServices) return;

        C_BaseViewModel* pViewModel = I::GameResourceService->pGameEntitySystem->Get<C_BaseViewModel>(pViewModelServices->m_hViewModel());
        if (!pViewModel)
            return;

        // Create a struct to store glove information
        addedGloves.itemId = pItemViewToEquip->m_iItemID();
        addedGloves.itemHighId = pItemViewToEquip->m_iItemIDHigh();
        addedGloves.itemLowId = pItemViewToEquip->m_iItemIDLow();
        addedGloves.itemDefId = pItemViewToEquip->m_iItemDefinitionIndex();

        C_EconItemView* pGloves = &pLocalPawn->m_EconGloves();
        if (!pGloves)
            return;

        CEconItemDefinition* pGlovesDefinition = pGloves->GetStaticData();
        if (!pGlovesDefinition)
            return;

        skin_changer::OnGlove(pInventory, pLocalPawn, pViewModel, pGloves, pGlovesDefinition);
        pLocalPawn->m_bNeedToReApplyGloves() = true;
        pInventory->SOUpdated(pInventory->GetOwner(), (CSharedObject*)pItemInLoadoutSOCData, eSOCacheEvent_Incremental);
        return;
    }
    else if (toequipdata->IsKnife(false, pItemInLoadoutStaticData->m_pszItemTypeName)) {
        pInventory->SOUpdated(pInventory->GetOwner(), (CSharedObject*)pItemInLoadoutSOCData, eSOCacheEvent_Incremental);
        return;
    }

}

void skin_changer::OnSetModel(C_BaseModelEntity* pEntity, const char*& model) {
    // When you're lagging you may see the default knife for one second and this
    // function fixes that.
    if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
        return;

    CCSPlayerController* pLocalPlayerController = CCSPlayerController::GetLocalPlayerController();
    if (!pLocalPlayerController) return;

    C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalPlayerController->GetPawnHandle());
    if (!pLocalPawn) return;

    if (pLocalPawn->GetHealth() <= 0)
        return;

    if (!pEntity || !pEntity->IsViewModel()) return;

    C_BaseViewModel* pViewModel = (C_BaseViewModel*)pEntity;
    if (!pViewModel)
        return;
    CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
    if (!pInventory) return;

    const uint64_t steamID = pInventory->GetOwner().m_id;

    C_CSWeaponBase* pWeapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(pViewModel->m_hWeapon());
    if (!pWeapon) return;

    if (!pWeapon || !pWeapon->IsWeapon() ||
        pWeapon->GetOriginalOwnerXuid() != steamID)
        return;

    CAttributeManager* pAttributeContainer = &pWeapon->m_AttributeManager();
    if (!pAttributeContainer) return;


    C_EconItemView* pWeaponItemView = &pAttributeContainer->m_Item();
    if (!pWeaponItemView) return;

    CEconItemDefinition* pWeaponDefinition = pWeaponItemView->GetStaticData();
    if (!pWeaponDefinition) return;

    C_EconItemView* pWeaponInLoadoutItemView = pInventory->GetItemInLoadout(
        pWeapon->m_iOriginalTeamNumber(), pWeaponDefinition->GetLoadoutSlot());
    if (!pWeaponInLoadoutItemView) return;

    // Check if skin is added by us.
    auto it = std::find(g_vecAddedItemsIDs.cbegin(), g_vecAddedItemsIDs.cend(),
        pWeaponInLoadoutItemView->m_iItemID());
    if (it == g_vecAddedItemsIDs.cend()) return;

    CEconItemDefinition* pWeaponInLoadoutDefinition =
        pWeaponInLoadoutItemView->GetStaticData();

    if (!pWeaponInLoadoutDefinition)// ||
        return;

    model = pWeaponInLoadoutDefinition->GetModelName();
}

void skin_changer::AddEconItemToList(CEconItem* pItem) {
    g_vecAddedItemsIDs.emplace_back(pItem->m_ulID);
}

void skin_changer::Shutdown() {
    CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
    if (!pInventory) return;

    for (uint64_t id : g_vecAddedItemsIDs) {
        pInventory->RemoveEconItem(pInventory->GetSOCDataForItem(id));
    }
}

void skin_changer::OnRoundReset(IGameEvent* pEvent) {
    if (!pEvent || g_vecAddedItemsIDs.empty()) return;

    const char* eventName = pEvent->GetName();
    if (!eventName) return;

    CCSPlayerController* pLocalPlayerController = CCSPlayerController::GetLocalPlayerController();

    if (!pLocalPlayerController )
        return;

    C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalPlayerController->GetPawnHandle());
    if (!pLocalPawn) return;

}

void skin_changer::OnPreFireEvent(IGameEvent* pEvent) {
    if (!pEvent) return;

    const char* eventName = pEvent->GetName();
    if (!eventName) return;

    const auto pControllerWhoKilled = pEvent->get_player_controller("attacker");
    if (pControllerWhoKilled == nullptr)
        return;

    const auto pControllerWhoDied = pEvent->get_player_controller("userid");
    if (pControllerWhoDied == nullptr)
        return;

    if (pControllerWhoKilled->GetIdentity()->GetIndex() == pControllerWhoDied->GetIdentity()->GetIndex())
        return;

    CCSPlayerController* pLocalPlayerController = CCSPlayerController::GetLocalPlayerController();

    if (!pLocalPlayerController || pControllerWhoKilled->GetIdentity()->GetIndex() != pLocalPlayerController->GetIdentity()->GetIndex())
        return;

    C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalPlayerController->GetPawnHandle());
    if (!pLocalPawn) return;

    CPlayer_WeaponServices* pWeaponServices = pLocalPawn->GetWeaponServices();
    if (!pWeaponServices) return;

    C_CSWeaponBase* pActiveWeapon = pLocalPawn->ActiveWeapon();
    if (!pActiveWeapon) return;

    CAttributeManager* pAttributeContainer = &pActiveWeapon->m_AttributeManager();
    if (!pAttributeContainer) return;

    C_EconItemView* pWeaponItemView = &pAttributeContainer->m_Item();
    if (!pWeaponItemView) return;

    CEconItemDefinition* pWeaponDefinition = pWeaponItemView->GetStaticData();
    if (!pWeaponDefinition || !pWeaponDefinition->IsKnife(true, pWeaponDefinition->m_pszItemTypeName)) return;
    const std::string_view token_name = CS_XOR("weapon");
    CUtlStringToken token(token_name.data());

    pEvent->SetString(token, pWeaponDefinition->GetSimpleWeaponName());
}