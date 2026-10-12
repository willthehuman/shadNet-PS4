// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdint>
#include <cstdio>
#include <cstring>
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
    // Bridge type-6 P2P datagram sockets to plain UDP at the library level too:
    // libSceNet does P2P-specific setup on the fd that a bridged kernel socket
    // cannot satisfy (the game retried creation 7x in the trace). Passing type 2
    // straight through gives the game a consistent, working datagram socket.
    s32 effective_type = type;
    if (family == 2 && type == 6) {
        effective_type = 2; // SOCK_DGRAM
    }
    if (IsPlausibleString(name)) {
        LOG_INFO(Lib_Net, "called, name = {}, family = {}, type = {}{}, protocol = {}", name, family,
                 type, type != effective_type ? " -> bridged to DGRAM" : "", protocol);
    } else {
        LOG_WARNING(Lib_Net,
                    "called, name = <bad ptr {:#x}>, family = {}, type = {}{}, protocol = {}",
                    reinterpret_cast<std::uintptr_t>(name), family, type,
                    type != effective_type ? " -> bridged to DGRAM" : "", protocol);
    }
    return SHADNET_HOOK_CONTINUE(sceNetSocket, name, family, effective_type, protocol);
}

HOOK_INIT(sceNetResolverStartNtoa);
s32 sceNetResolverStartNtoa_hook(s32 rid, const char* hostname, OrbisNetInAddr* addr, s32 timeout,
                                 s32 retry, s32 flags) {
    if (!IsPlausibleString(hostname)) {
        // Never forward a junk pointer into the real resolver — it dereferences it.
        // RAW X-RAY: dump everything needed to identify this call offline —
        // the caller's code bytes, the top of the stack, and any argument that
        // points into the low mapped window read as a printable string.
        const auto caller = reinterpret_cast<std::uintptr_t>(__builtin_return_address(0));
        char codebytes[97]{};
        if (caller > 64) {
            const auto* p = reinterpret_cast<const u8*>(caller - 48);
            for (int i = 0; i < 48; ++i) {
                std::snprintf(codebytes + i * 2, 3, "%02x", p[i]);
            }
        }
        LOG_WARNING(Lib_Net, "raw rid={:#x} hostname={:#x} addr={:#x} timeout={:#x} retry={:#x} "
                             "flags={:#x} caller={:#x} code48={}",
                    static_cast<std::uint32_t>(rid), reinterpret_cast<std::uintptr_t>(hostname),
                    reinterpret_cast<std::uintptr_t>(addr), static_cast<std::uint32_t>(timeout),
                    static_cast<std::uint32_t>(retry), static_cast<std::uint32_t>(flags), caller,
                    codebytes);
        std::uintptr_t stack[16]{};
        std::uintptr_t sp = 0;
        asm volatile("movq %%rsp, %0" : "=r"(sp));
        std::memcpy(stack, reinterpret_cast<void*>(sp), sizeof(stack));
        LOG_WARNING(Lib_Net, "raw stack: {:#x} {:#x} {:#x} {:#x} {:#x} {:#x} {:#x} {:#x} {:#x} "
                             "{:#x} {:#x} {:#x} {:#x} {:#x} {:#x} {:#x}",
                    stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6], stack[7],
                    stack[8], stack[9], stack[10], stack[11], stack[12], stack[13], stack[14],
                    stack[15]);
        const std::uintptr_t args[6] = {static_cast<std::uint32_t>(rid),
                                        reinterpret_cast<std::uintptr_t>(hostname),
                                        reinterpret_cast<std::uintptr_t>(addr),
                                        static_cast<std::uint32_t>(timeout),
                                        static_cast<std::uint32_t>(retry),
                                        static_cast<std::uint32_t>(flags)};
        for (int i = 0; i < 6; ++i) {
            // Only probe the low window both modules live in — that range is
            // certainly mapped, so reading is safe. Anything else: skip.
            if (args[i] >= 0x800000000ull && args[i] < 0x801000000ull) {
                const char* s = reinterpret_cast<const char*>(args[i]);
                char probe[33]{};
                int n = 0;
                for (; n < 32 && s[n] != '\0'; ++n) {
                    const char c = s[n];
                    if (c < 0x20 || c > 0x7e) {
                        n = -1;
                        break;
                    }
                }
                if (n >= 3) {
                    std::memcpy(probe, s, static_cast<size_t>(n));
                    LOG_WARNING(Lib_Net, "raw arg[{}] as string: {}", i, probe);
                }
            }
        }
        return kSceNetErrorEINVAL;
    }
    LOG_INFO(Lib_Net, "called, hostname = {}, timeout = {}, retry = {}", hostname, timeout, retry);
    return SHADNET_HOOK_CONTINUE(sceNetResolverStartNtoa, rid, hostname, addr, timeout, retry,
                                 flags);
}

void RegisterNetHooks() {
    HOOK(sceNetSocket);
    HOOK(sceNetResolverStartNtoa);
    // NOTE (2026-10-12): do NOT detour sceNetResolverCreate/Destroy. A build that
    // registered them made the plugin's own first resolver call return 0x804101C8
    // (the lazily-bound import breaks when its first call lands on a fresh
    // detour). The socket-level decode trace covers the same ground safely.
}

namespace Libraries::Network::Net {

void RegisterHooks() {
    return RegisterNetHooks();
}
} // namespace Libraries::Network::Net
