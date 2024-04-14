#pragma once

#include "../cstrike/utilities/memory.h"
class CCSPlayerInventory;

class CCSInventoryManager {
   public:
    static CCSInventoryManager* GetInstance();

    auto EquipItemInLoadout(int iTeam, int iSlot, uint64_t iItemID) {
        return MEM::CallVFunc<bool, 54u>( this, iTeam, iSlot, iItemID);
    }
    auto GetLocalInventory() {
        return MEM::CallVFunc<CCSPlayerInventory*, 57u>(this);
    }

};
