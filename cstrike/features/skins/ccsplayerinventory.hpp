#pragma once

#include <utility>


#include "../cstrike/sdk/entity.h"
#include "../cstrike/sdk/datatypes/utlvector.h"


#include "ccsinventorymanager.hpp"
#include <cstdint>


class CCSPlayerInventory {
   public:
    static CCSPlayerInventory* GetInstance();

    auto SOCreated(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent) {
        return MEM::CallVFunc<void, 0u>(this, owner, pObject, eEvent);
    }

    auto SOUpdated(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent) {
        return MEM::CallVFunc<void, 1u>(this, owner, pObject, eEvent);

    }

    auto SODestroyed(SOID_t owner, CSharedObject* pObject,  ESOCacheEvent eEvent) {
        return MEM::CallVFunc<void, 2u>(this, owner, pObject, eEvent);
    }

    auto GetItemInLoadout(int iClass, int iSlot) {
        return MEM::CallVFunc<C_EconItemView*,8u>(this, iClass, iSlot);
    }

    bool AddEconItem(CEconItem* pItem);
    void RemoveEconItem(CEconItem* pItem);
    std::pair<uint64_t, uint32_t> GetHighestIDs();
    C_EconItemView* GetItemViewForItem(uint64_t itemID);
    CEconItem* GetSOCDataForItem(uint64_t itemID);

    auto GetOwner() {
        return *reinterpret_cast<SOID_t*>((uintptr_t)(this) + 0x10);
    }

    auto& GetItemVector() {
        return *reinterpret_cast<CUtlVector<C_EconItemView*>*>(
            (uintptr_t)(this) + 0x20);
    }
};
