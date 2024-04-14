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
#include <cstring>
class ShadowVMT
{
public:
	ShadowVMT();
	ShadowVMT(void* base);
	~ShadowVMT();

	bool Setup(void* class_base = nullptr);

	template<typename T>
	void HookIndex(int index, T fun)
	{
		new_vftbl[index + 1] = reinterpret_cast<std::uintptr_t>(fun);
	}
	void UnhookIndex(int index)
	{
		new_vftbl[index] = old_vftbl[index];
	}
	void UnhookAll()
	{
		try {
			if (old_vftbl != nullptr) {
				DWORD old;
				VirtualProtect(class_base, sizeof(uintptr_t), PAGE_READWRITE, &old);
				*(std::uintptr_t**)class_base = old_vftbl;
				old_vftbl = nullptr;
				VirtualProtect(class_base, sizeof(uintptr_t), old, &old);
			}
		}
		catch (...) {
		}
	}

	template<typename T>
	T GetOg(int index)
	{
		return (T)old_vftbl[index];
	}

private:
	inline std::size_t CalcVtableLength(std::uintptr_t* vftbl_start);

	void*           class_base;
	std::size_t     vftbl_len;
	std::uintptr_t* new_vftbl;
	std::uintptr_t* old_vftbl;
};