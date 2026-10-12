// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdint>
#include <orbis/Net.h>

#include "common/logging/log.h"
#include "core/libraries/network/net.h"

// Games and system libraries can hand hooked net functions short junk pointers
// (observed: hostname = 0x2) where a C-string is expected. fmt calls strlen()
// on such arguments, so guard every game-supplied string before it reaches a
// log — anything below 0x10000 is not a real pointer. (CE-34878-0 fix, r2.)
static inline bool IsPlausibleString(const char* p) {
    return reinterpret_cast<std::uintptr_t>(p) >= 0x10000;
}

// sceNet error numbers are 0x80410100 + errno; EINVAL = 22.
static constexpr s32 kSceNetErrorEINVAL = 0x80410116;

HOOK_INIT(sceNetSocket);
s32 sceNetSocket_hook(const char* name, s32 family, s32 type, s32 protocol) {
    if (IsPlausibleString(name)) {
        LOG_INFO(Lib_Net, "called, name = {}, family = {}, type = {}, protocol = {}", name, family,
                 type, protocol);
    } else {
        LOG_WARNING(Lib_Net, "called, name = <bad ptr {:#x}>, family = {}, type = {}, protocol = {}",
                    reinterpret_cast<std::uintptr_t>(name), family, type, protocol);
    }
    return SHADNET_HOOK_CONTINUE(sceNetSocket, name, family, type, protocol);
}

HOOK_INIT(sceNetResolverStartNtoa);
s32 sceNetResolverStartNtoa_hook(s32 rid, const char* hostname, OrbisNetInAddr* addr, s32 timeout,
                                 s32 retry, s32 flags) {
    if (!IsPlausibleString(hostname)) {
        // Never forward a junk pointer into the real resolver — it dereferences it.
        LOG_WARNING(Lib_Net,
                    "called, hostname = <bad ptr {:#x}>, rid={}, timeout={}, retry={}, flags={}, "
                    "caller={:#x} -> returning EINVAL",
                    reinterpret_cast<std::uintptr_t>(hostname), rid, timeout, retry, flags,
                    reinterpret_cast<std::uintptr_t>(__builtin_return_address(0)));
        return kSceNetErrorEINVAL;
    }
    LOG_INFO(Lib_Net, "called, hostname = {}, timeout = {}, retry = {}", hostname, timeout, retry);
    return SHADNET_HOOK_CONTINUE(sceNetResolverStartNtoa, rid, hostname, addr, timeout, retry,
                                 flags);
}

void RegisterNetHooks() {
    HOOK(sceNetSocket);
    HOOK(sceNetResolverStartNtoa);
}

namespace Libraries::Network::Net {

void RegisterHooks() {
    return RegisterNetHooks();
}
} // namespace Libraries::Network::Net
