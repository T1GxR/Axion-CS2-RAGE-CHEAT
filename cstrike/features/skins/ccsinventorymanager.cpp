#include "ccsinventorymanager.hpp"

#include "../cstrike/utilities/memory.h"
#include "../cstrike/core/interfaces.h"
#include "../cstrike/core/hooks.h"
CCSInventoryManager* CCSInventoryManager::GetInstance() {
    if (!H::fnGetInventoryManager) return nullptr;
    return H::fnGetInventoryManager();


}