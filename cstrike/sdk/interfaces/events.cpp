#include "events.h"

std::int64_t IGameEvent::get_int(const std::string_view event_name) noexcept {
    // client.dll; 48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 30 48 8B 01 41 8B F0 4C 8B F1 41 B0 01 48 8D 4C 24 20 48 8B DA 48 8B 78
    using function_t = std::int64_t(__fastcall*)(void*, const char*);
    static function_t fn = reinterpret_cast<function_t>(MEM::FindPattern(CLIENT_DLL, "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 30 48 8B 01 41 8B F0 4C 8B F1 41 B0 01 48 8D 4C 24 20 48 8B DA 48 8B 78"));
    CS_ASSERT(fn != nullptr);
    return fn(this, event_name.data());
}

void* IGameEvent::get_player_pawn_from_id(const std::string_view event_name) noexcept {
    // client.dll; 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 01 48 8B F1 41 B0 01 48 8D 4C 24 20 48 8B FA 48 8B 98
    using function_t = void*(__fastcall*)(void*, const char*, std::int64_t);
    static function_t fn = reinterpret_cast<function_t>(MEM::FindPattern(CLIENT_DLL, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 01 48 8B F1 41 B0 01 48 8D 4C 24 20 48 8B FA 48 8B 98"));
    CS_ASSERT(fn != nullptr);
    return fn(this, event_name.data(), 0);
}

void* IGameEvent::get_pointer_from_id(const std::string_view event_name) noexcept {
    // used: "userid", "attackerid"
    std::int64_t id{ };
    {
        // client.dll; 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 48 8B 01 49
        using function_t = std::int64_t(__fastcall*)(void*, std::int64_t*, const char*);
        static function_t fn = reinterpret_cast<function_t>(MEM::FindPattern(CLIENT_DLL, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 01 48 8B F1 41 B0 01 48 8D 4C 24 20 48 8B FA 48 8B 98"));
        CS_ASSERT(fn != nullptr);
        fn(this, &id, event_name.data());
    }

    if (id == -1)
        return { };

    // xref: "player_disconnect"
    // client.dll; E8 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8B D8 48 85 C9
    using function_t2 = void*(__fastcall*)(std::int64_t);
    static function_t2 fn = reinterpret_cast<function_t2>(MEM::FindPattern(CLIENT_DLL, "E8 ?? ?? ?? ?? 48 8B 0D ?? ?? ?? ?? 48 8B D8 48 85 C9"));
    CS_ASSERT(fn != nullptr);

    return fn(id);
}