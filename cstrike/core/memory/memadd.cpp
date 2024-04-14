

#include "../memory/memadd.h"
#include "../csig/sigscan.hpp"
#include "../fnv1a.hpp"
void CMemory::Initialize() {

    for (CSigScan* it : m_ScheduledScans) {
        it->FindSignature();
        it->FreeData();
    }

    std::vector<CSigScan*>().swap(m_ScheduledScans);
}

ModulePtr_t& CMemory::GetModuleInternal(const char* libName) {
    auto hash = fnv1a::Hash(libName);

    auto it = m_CachedModules.find(hash);
    if (it != m_CachedModules.end()) {
        return it->second;
    }

    auto module = std::make_unique<CModule>(libName);
    if (module->Retrieve()) {
        return m_CachedModules.emplace(hash, std::move(module)).first->second;
    }


    static ModulePtr_t null{};
    return null;
}

CPointer CMemory::GetInterfaceInternal(const char* libName, const char* version) {
    CPointer rv = 0;

    auto& library = CMemory::GetModuleInternal(libName);
    if (!library) {
        return rv;
    }

    rv = library->GetInterface(version);

    return rv;
}

CPointer CMemory::GetProcAddressInternal(const char* libName, const char* procName) {
    CPointer rv = 0;

    auto& library = CMemory::GetModuleInternal(libName);
    if (!library) {
        return rv;
    }

    rv = library->GetProcAddress(procName);

    return rv;
}
