#pragma once

#include <Windows.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <array>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>
#include <chrono>
#include <random>
#include <numeric>
#include <cstdint>
#include <memory>
#include <thread>
#include <type_traits>
#include <regex>
#include <cmath>
#include <fstream>
#include <cassert>
#include <process.h>
#include <DbgHelp.h>
#include <filesystem>
#include <libloaderapi.h>
#include <Psapi.h>
#include <corecrt_math_defines.h>
#include <numbers>
#include <iomanip>
#include <iosfwd>
#include <set>
#include <unordered_set>
#include <list>
#include <TlHelp32.h>
#include <cinttypes>
#include <cstring>
class InlineHook
{
    std::vector<BYTE> og_bytes;
    DWORD original = 0;
    DWORD source = 0;
    bool bEnabled = false;
public:
    InlineHook(){}

    void Hook(void* src, void* dest, const size_t len);
    void Unhook();

    template<typename T>
    T GetOg()
    {
        return (T)original;
    }
};
