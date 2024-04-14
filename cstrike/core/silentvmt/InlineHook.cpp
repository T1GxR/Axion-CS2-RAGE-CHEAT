#include "InlineHook.h"
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <memoryapi.h>
#include "../cstrike/core/spoofcall/lazy_importer.hpp"
bool detour(BYTE* src, BYTE* dst, const uintptr_t len)
{
	if (len < 5) return false;
	DWORD curProtection;
	LI_FN(VirtualProtect).safe()(src, len, PAGE_EXECUTE_READWRITE, &curProtection);
	memset(src, 0x90, len);
	uintptr_t relativeAddress = ((uintptr_t)dst - (uintptr_t)src) - 5;
	*(BYTE*)src = 0xE9;
	*(uintptr_t*)((uintptr_t)src + 1) = relativeAddress;
	DWORD temp;
	LI_FN(VirtualProtect).safe()(src, len, curProtection, &temp);
	return true;
}

BYTE* trampHook(BYTE* src, BYTE* dst, const uintptr_t len)
{
    if (len < 5) return 0;
    void* gateway = LI_FN(VirtualAlloc).safe()(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memcpy(gateway, src, len);
    intptr_t  gatewayRelativeAddr = ((intptr_t)src - (intptr_t)gateway) - 5;
    *(char*)((intptr_t)gateway + len) = 0xE9;
    *(intptr_t*)((intptr_t)gateway + len + 1) = gatewayRelativeAddr;
    // detour(src, dst, len);
    return (BYTE*) gateway;
}

void InlineHook::Hook(void* src, void* dest, const size_t len)
{
    const BYTE* src_bytes = (BYTE*) src;
    
    for(int i = 0; i < len; i++)
        og_bytes.push_back(src_bytes[i]);

    source = (DWORD) src;
    
    original = (DWORD)trampHook((BYTE*) src, (BYTE*) dest, len);

    if(original)
        bEnabled = true;
}

void InlineHook::Unhook()
{
    BYTE* bytes = (BYTE*) source;

    int i = 0;
    for(const BYTE& b : og_bytes)
        bytes[i++] = b;
}