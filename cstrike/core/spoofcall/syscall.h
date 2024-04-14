#pragma once
#ifndef DIRECT_SYSCALL_HPP
#define DIRECT_SYSCALL_HPP

#include <cstdint>
#include <string>
#include <windows.h>

#ifndef SYSCALL_NO_FORCEINLINE
#if defined(_MSC_VER)
#define SYSCALL_FORCEINLINE __forceinline
#endif
#else
#define SYSCALL_FORCEINLINE inline
#endif

#include <intrin.h>
#include <memory>
#include <vector>

#define SYSCALL_HASH_CT(str)                                            \
    []() [[msvc::forceinline]] {                                        \
        constexpr uint32_t hash_out{::syscall::fnv1a::hash_ctime(str)}; \
                                                                        \
        return hash_out;                                                \
    }()

#define SYSCALL_HASH(str) ::syscall::fnv1a::hash_rtime(str)

#define INVOKE_LAZY_FN(type, export_name, ...)                                      \
    [&]() [[msvc::forceinline]] {                                                   \
        constexpr uint32_t export_hash{::syscall::fnv1a::hash_ctime(#export_name)}; \
                                                                                    \
        return syscall::invoke_lazy_import<type>(export_hash, __VA_ARGS__);         \
    }()

#define INVOKE_SYSCALL(type, export_name, ...)                                      \
    [&]() [[msvc::forceinline]] {                                                   \
        constexpr uint32_t export_hash{::syscall::fnv1a::hash_ctime(#export_name)}; \
                                                                                    \
        return syscall::invoke_syscall<type>(export_hash, __VA_ARGS__);             \
    }()

namespace syscall {
    namespace nt {
        typedef struct _PEB_LDR_DATA {
            ULONG      Length;
            BOOLEAN    Initialized;
            PVOID      SsHandle;
            LIST_ENTRY InLoadOrderModuleList;
            LIST_ENTRY InMemoryOrderModuleList;
            LIST_ENTRY InInitializationOrderModuleList;
        } PEB_LDR_DATA, * PPEB_LDR_DATA;

        struct UNICODE_STRING {
            uint16_t Length;
            uint16_t MaximumLength;
            wchar_t* Buffer;
        };

        typedef struct _LDR_MODULE {
            LIST_ENTRY     InLoadOrderModuleList;
            LIST_ENTRY     InMemoryOrderModuleList;
            LIST_ENTRY     InInitializationOrderModuleList;
            PVOID          BaseAddress;
            PVOID          EntryPoint;
            ULONG          SizeOfImage;
            UNICODE_STRING FullDllName;
            UNICODE_STRING BaseDllName;
            ULONG          Flags;
            SHORT          LoadCount;
            SHORT          TlsIndex;
            LIST_ENTRY     HashTableEntry;
            ULONG          TimeDateStamp;
        } LDR_MODULE, * PLDR_MODULE;

        typedef struct _PEB_FREE_BLOCK {
            _PEB_FREE_BLOCK* Next;
            ULONG            Size;
        } PEB_FREE_BLOCK, * PPEB_FREE_BLOCK;

        typedef struct _LDR_DATA_TABLE_ENTRY {
            LIST_ENTRY     InLoadOrderLinks;
            LIST_ENTRY     InMemoryOrderLinks;
            PVOID          Reserved2[2];
            PVOID          DllBase;
            PVOID          EntryPoint;
            PVOID          Reserved3;
            UNICODE_STRING FullDllName;
            UNICODE_STRING BaseDllName;
            PVOID          Reserved5[3];
            union {
                ULONG      CheckSum;
                PVOID      Reserved6;
            };
            ULONG          TimeDateStamp;
        } LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

        typedef struct _RTL_DRIVE_LETTER_CURDIR {
            USHORT         Flags;
            USHORT         Length;
            ULONG          TimeStamp;
            UNICODE_STRING DosPath;
        } RTL_DRIVE_LETTER_CURDIR, * PRTL_DRIVE_LETTER_CURDIR;

        typedef struct _RTL_USER_PROCESS_PARAMETERS {
            ULONG                   MaximumLength;
            ULONG                   Length;
            ULONG                   Flags;
            ULONG                   DebugFlags;
            PVOID                   ConsoleHandle;
            ULONG                   ConsoleFlags;
            HANDLE                  StdInputHandle;
            HANDLE                  StdOutputHandle;
            HANDLE                  StdErrorHandle;
            UNICODE_STRING          CurrentDirectoryPath;
            HANDLE                  CurrentDirectoryHandle;
            UNICODE_STRING          DllPath;
            UNICODE_STRING          ImagePathName;
            UNICODE_STRING          CommandLine;
            PVOID                   Environment;
            ULONG                   StartingPositionLeft;
            ULONG                   StartingPositionTop;
            ULONG                   Width;
            ULONG                   Height;
            ULONG                   CharWidth;
            ULONG                   CharHeight;
            ULONG                   ConsoleTextAttributes;
            ULONG                   WindowFlags;
            ULONG                   ShowWindowFlags;
            UNICODE_STRING          WindowTitle;
            UNICODE_STRING          DesktopName;
            UNICODE_STRING          ShellInfo;
            UNICODE_STRING          RuntimeData;
            RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
        } RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

        typedef struct _PEB {
            BOOLEAN                     InheritedAddressSpace;
            BOOLEAN                     ReadImageFileExecOptions;
            BOOLEAN                     BeingDebugged;
            BOOLEAN                     Spare;
            HANDLE                      Mutant;
            PVOID                       ImageBaseAddress;
            PPEB_LDR_DATA               LoaderData;
            RTL_USER_PROCESS_PARAMETERS ProcessParameters;
            PVOID                       SubSystemData;
            PVOID                       ProcessHeap;
            PVOID                       FastPebLock;
            uintptr_t                   FastPebLockRoutine;
            uintptr_t                   FastPebUnlockRoutine;
            ULONG                       EnvironmentUpdateCount;
            uintptr_t                   KernelCallbackTable;
            PVOID                       EventLogSection;
            PVOID                       EventLog;
            PPEB_FREE_BLOCK             FreeList;
            ULONG                       TlsExpansionCounter;
            PVOID                       TlsBitmap;
            ULONG                       TlsBitmapBits[0x2];
            PVOID                       ReadOnlySharedMemoryBase;
            PVOID                       ReadOnlySharedMemoryHeap;
            uintptr_t                   ReadOnlyStaticServerData;
            PVOID                       AnsiCodePageData;
            PVOID                       OemCodePageData;
            PVOID                       UnicodeCaseTableData;
            ULONG                       NumberOfProcessors;
            ULONG                       NtGlobalFlag;
            BYTE                        Spare2[0x4];
            LARGE_INTEGER               CriticalSectionTimeout;
            ULONG                       HeapSegmentReserve;
            ULONG                       HeapSegmentCommit;
            ULONG                       HeapDeCommitTotalFreeThreshold;
            ULONG                       HeapDeCommitFreeBlockThreshold;
            ULONG                       NumberOfHeaps;
            ULONG                       MaximumNumberOfHeaps;
            uintptr_t* ProcessHeaps;
            PVOID                       GdiSharedHandleTable;
            PVOID                       ProcessStarterHelper;
            PVOID                       GdiDCAttributeList;
            PVOID                       LoaderLock;
            ULONG                       OSMajorVersion;
            ULONG                       OSMinorVersion;
            ULONG                       OSBuildNumber;
            ULONG                       OSPlatformId;
            ULONG                       ImageSubSystem;
            ULONG                       ImageSubSystemMajorVersion;
            ULONG                       ImageSubSystemMinorVersion;
            ULONG                       GdiHandleBuffer[0x22];
            ULONG                       PostProcessInitRoutine;
            ULONG                       TlsExpansionBitmap;
            BYTE                        TlsExpansionBitmapBits[0x80];
            ULONG                       SessionId;
        } PEB, * PPEB;
    }// namespace nt

    constexpr uint32_t xor_key_1 = __TIME__[2];
    constexpr uint32_t xor_key_2 = __TIME__[4];
    constexpr uint32_t xor_key_offset = (xor_key_1 ^ xor_key_2);

    namespace fnv1a {
        constexpr uint32_t fnv_prime_value = 0x01000193;

        SYSCALL_FORCEINLINE consteval uint32_t hash_ctime(const char* input, unsigned val = 0x811c9dc5 ^ ::syscall::xor_key_offset) noexcept
        {
            return input[0] == CS_XOR('\0') ? val : hash_ctime(input + 1, (val ^ *input) * fnv_prime_value);
        }

        SYSCALL_FORCEINLINE constexpr uint32_t hash_rtime(const char* input, unsigned val = 0x811c9dc5 ^ ::syscall::xor_key_offset) noexcept
        {
            return input[0] == CS_XOR('\0') ? val : hash_rtime(input + 1, (val ^ *input) * fnv_prime_value);
        }
    }// namespace fnv1a

    namespace utils {
        SYSCALL_FORCEINLINE std::string wide_to_string(wchar_t* buffer) noexcept
        {
            const auto out{ std::wstring(buffer) };

            if (out.empty())
                return "";

            return std::string(out.begin(), out.end());
        }
    }// namespace utils

    namespace win {
        SYSCALL_FORCEINLINE nt::PEB* get_peb() noexcept
        {
#if defined(_M_IX86) || defined(__i386__)
            return reinterpret_cast<::syscall::nt::PEB*>(__readfsdword(0x30));
#else
            return reinterpret_cast<::syscall::nt::PEB*>(__readgsqword(0x60));
#endif
        }

        template<typename T>
        static SYSCALL_FORCEINLINE T get_module_handle_from_hash(const uint32_t& module_hash) noexcept
        {
            auto peb = ::syscall::win::get_peb();

            if (!peb)
                return NULL;

            auto head = &peb->LoaderData->InLoadOrderModuleList;

            for (auto it = head->Flink; it != head; it = it->Flink) {
                ::syscall::nt::_LDR_DATA_TABLE_ENTRY* ldr_entry = CONTAINING_RECORD(it, nt::LDR_DATA_TABLE_ENTRY,
                    InLoadOrderLinks);

                if (!ldr_entry->BaseDllName.Buffer)
                    continue;

                auto name = ::syscall::utils::wide_to_string(ldr_entry->BaseDllName.Buffer);

                if (SYSCALL_HASH(name.data()) == module_hash)
                    return reinterpret_cast<T>(ldr_entry->DllBase);
            }

            return NULL;
        }

        template<typename T>
        static SYSCALL_FORCEINLINE T get_module_export_from_table(uintptr_t module_address,
            const uint32_t& export_hash) noexcept
        {
            auto dos_headers = reinterpret_cast<IMAGE_DOS_HEADER*>(module_address);

            if (dos_headers->e_magic != IMAGE_DOS_SIGNATURE)
                return NULL;

            PIMAGE_EXPORT_DIRECTORY export_directory = nullptr;

            auto nt_headers32 = reinterpret_cast<PIMAGE_NT_HEADERS32>(module_address + dos_headers->e_lfanew);
            auto nt_headers64 = reinterpret_cast<PIMAGE_NT_HEADERS64>(module_address + dos_headers->e_lfanew);

            PIMAGE_OPTIONAL_HEADER32 optional_header32 = &nt_headers32->OptionalHeader;
            PIMAGE_OPTIONAL_HEADER64 optional_header64 = &nt_headers64->OptionalHeader;

            // for 32bit modules.
            if (nt_headers32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                // does not have a export table.
                if (optional_header32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size <= 0U)
                    return NULL;

                export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(module_address + optional_header32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
            }
            // for 64bit modules.
            else if (nt_headers64->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                // does not have a export table.
                if (optional_header64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size <= 0U)
                    return NULL;

                export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(module_address + optional_header64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
            }

            auto names_rva = reinterpret_cast<uint32_t*>(module_address + export_directory->AddressOfNames);
            auto functions_rva = reinterpret_cast<uint32_t*>(module_address + export_directory->AddressOfFunctions);
            auto name_ordinals = reinterpret_cast<unsigned short*>(module_address + export_directory->AddressOfNameOrdinals);

            uint32_t number_of_names = export_directory->NumberOfNames;

            for (size_t i = 0ul; i < number_of_names; i++) {
                const char* export_name = reinterpret_cast<const char*>(module_address + names_rva[i]);

                if (export_hash == SYSCALL_HASH(export_name))
                    return static_cast<T>(module_address + functions_rva[name_ordinals[i]]);
            }

            return NULL;
        }

        template<typename T>
        SYSCALL_FORCEINLINE T force_find_export(const uint32_t& export_hash) noexcept
        {
            auto peb = ::syscall::win::get_peb();

            if (!peb || !export_hash)
                return NULL;

            auto head = &peb->LoaderData->InLoadOrderModuleList;

            for (auto it = head->Flink; it != head; it = it->Flink) {
                ::syscall::nt::_LDR_DATA_TABLE_ENTRY* ldr_entry = CONTAINING_RECORD(it,
                    nt::LDR_DATA_TABLE_ENTRY,
                    InLoadOrderLinks);

                if (!ldr_entry->BaseDllName.Buffer)
                    continue;

                auto name = ::syscall::utils::wide_to_string(ldr_entry->BaseDllName.Buffer);

                auto export_address = ::syscall::win::get_module_export_from_table<uintptr_t>(
                    reinterpret_cast<uintptr_t>(ldr_entry->DllBase),
                    export_hash);

                if (!export_address)
                    continue;

                return static_cast<T>(export_address);
            }
        }
    }// namespace win

    SYSCALL_FORCEINLINE uint16_t get_return_code_from_export(uintptr_t export_address) noexcept
    {
        if (!export_address)
            return NULL;

        return *reinterpret_cast<int*>(static_cast<uintptr_t>(export_address + 12) + 1);
    }

    SYSCALL_FORCEINLINE int get_syscall_id_from_export(uintptr_t export_address) noexcept
    {
        if (!export_address)
            return NULL;

#if defined(_M_IX86) || defined(__i386__)
        return *reinterpret_cast<int*>(static_cast<uintptr_t>(export_address) + 1);
#else
        return *reinterpret_cast<int*>(static_cast<uintptr_t>(export_address + 3) + 1);
#endif
    }

    struct create_function {
        void* _allocated_memory = nullptr;
        void* _function = nullptr;
        uint32_t _export_hash;

    public:
        SYSCALL_FORCEINLINE ~create_function() noexcept
        {
            if (this->_allocated_memory) {
                VirtualFree(this->_allocated_memory, 0, MEM_RELEASE);
                this->_allocated_memory = nullptr;
            }
        }

        SYSCALL_FORCEINLINE create_function(uint32_t export_hash) noexcept
            : _export_hash(export_hash)
        {

            static auto exported_address = ::syscall::win::force_find_export<uintptr_t>(this->_export_hash);
            static auto syscall_table_id = ::syscall::get_syscall_id_from_export(exported_address);

            if (!exported_address || !syscall_table_id)
                return;

            std::vector<uint8_t> shellcode = {
#if defined(_M_IX86) || defined(__i386__)
                0xB8, 0x00, 0x10, 0x00, 0x00,            // mov eax, <syscall_id>
                0x64, 0x8B, 0x15, 0xC0, 0x00, 0x00, 0x00,// mov edx, DWORD PTR fs:0xc0 (
                0xFF, 0xD2,                              // call edx
                0xC2, 0x04, 0x00                         // ret 4
#else
                0x49, 0x89, 0xCA,                        // mov r10, rcx
                0xB8, 0x3F, 0x10, 0x00, 0x00,            // mov eax, <syscall_id>
                0x0F, 0x05,                              // syscall
                0xC3                                     // ret
#endif
            };

#if defined(_M_IX86) || defined(__i386__)
            // required for x86 ONLY!
            * reinterpret_cast<uint16_t*>(&shellcode[15]) = ::syscall::get_return_code_from_export(exported_address);
            *reinterpret_cast<int*>(&shellcode[1]) = syscall_table_id;
#else
            * reinterpret_cast<int*>(&shellcode[4]) = syscall_table_id;
#endif
            this->_allocated_memory = VirtualAlloc(nullptr,
                sizeof(shellcode),
                MEM_COMMIT | MEM_RESERVE,
                PAGE_EXECUTE_READWRITE);

            if (!this->_allocated_memory) {
                return;
            }

            memcpy(this->_allocated_memory, shellcode.data(), sizeof(shellcode));
            *reinterpret_cast<void**>(&this->_function) = this->_allocated_memory;
        }

        SYSCALL_FORCEINLINE bool is_valid_address() noexcept
        {
            return this->_function != nullptr;
        }

        template<typename T, typename... Args>
        SYSCALL_FORCEINLINE T invoke_call(Args... arguments) noexcept
        {
            return reinterpret_cast<T(__stdcall*)(Args...)>(this->_function)(arguments...);
        }
    };

    template<typename T, typename... Args>
    SYSCALL_FORCEINLINE T invoke_syscall(uint32_t export_hash, Args... arguments) noexcept
    {
        static auto syscall_function = ::syscall::create_function(export_hash);

        if (!syscall_function.is_valid_address()) {
            return NULL;
        }

        return syscall_function.invoke_call<T>(arguments...);
    }

    template<typename T, typename... Args>
    SYSCALL_FORCEINLINE T invoke_lazy_import(uint32_t export_hash, Args... arguments) noexcept
    {
        static auto exported_function = ::syscall::win::force_find_export<uintptr_t>(export_hash);

        if (exported_function)
            return reinterpret_cast<T(__stdcall*)(Args...)>(exported_function)(arguments...);
    }
}// namespace syscall

#endif// DIRECT_SYSCALL_HPP
