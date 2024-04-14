#include "ShadowVMT.h"
#include <cstdint>
#include <iostream>
#include <memoryapi.h>
#include "../spoofcall/lazy_importer.hpp"
ShadowVMT::ShadowVMT()
    : class_base(nullptr), vftbl_len(0), new_vftbl(nullptr), old_vftbl(nullptr)
{
}
ShadowVMT::ShadowVMT(void* base)
    : class_base(base), vftbl_len(0), new_vftbl(nullptr), old_vftbl(nullptr)
{
}
ShadowVMT::~ShadowVMT()
{
    UnhookAll();
}

bool ShadowVMT::Setup(void* base)
{
    if(base != nullptr)
        class_base = base;

    if(class_base == nullptr)
        return false;

    old_vftbl = *(std::uintptr_t**)class_base;
    vftbl_len = CalcVtableLength(old_vftbl) * sizeof(std::uintptr_t);

    if(vftbl_len == 0)
        return false;

    new_vftbl = new std::uintptr_t[vftbl_len + 1]();

    std::memcpy(&new_vftbl[1], old_vftbl, vftbl_len * sizeof(std::uintptr_t));

    try {
        DWORD old;
       LI_FN(VirtualProtect).safe()(class_base, sizeof(uintptr_t), PAGE_READWRITE, &old);
        new_vftbl[0] = old_vftbl[-1];
        *(std::uintptr_t**)class_base = &new_vftbl[1];
        LI_FN(VirtualProtect).safe()(class_base, sizeof(uintptr_t), old, &old);
    } catch(...) {
        delete[] new_vftbl;
        return false;
    }

    return true;
}
std::size_t ShadowVMT::CalcVtableLength(std::uintptr_t* vftbl_start)
{
    MEMORY_BASIC_INFORMATION memInfo = { NULL };
    int m_nSize = -1;
    do {
        m_nSize++;
        LI_FN( VirtualQuery).safe()(reinterpret_cast<LPCVOID>(vftbl_start[m_nSize]), &memInfo, sizeof(memInfo));
    } while (memInfo.Protect == PAGE_EXECUTE_READ || memInfo.Protect == PAGE_EXECUTE_READWRITE);

    return m_nSize;
}