// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <orbis/Net.h>

#include "common/hook_diag.h"
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

// v27 diag: how many sceNetSocket continuations are currently on this call
// stack. If the resolver junk-path fires while this is nonzero, the resolver
// hook was entered *from inside* a socket-call continuation (misrouted stub).
static std::uint32_t g_sock_continuation_depth = 0;

HOOK_INIT(sceNetSocket);
s32 sceNetSocket_hook(const char* name, s32 family, s32 type, s32 protocol) {
    // v27: capture entry context. The continuation call below runs the detour
    // stub — a verbatim copy of the real function's first bytes — so the entry
    // return address + stack pointer tell us exactly who reached the hooked
    // function and whether the result later shows up mangled.
    const auto entry_ret = reinterpret_cast<std::uintptr_t>(__builtin_return_address(0));
    std::uintptr_t entry_rsp = 0;
    asm volatile("movq %%rsp, %0" : "=r"(entry_rsp));
    ++g_sock_continuation_depth;
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
    const s32 result = SHADNET_HOOK_CONTINUE(sceNetSocket, name, family, effective_type, protocol);
    // v27: the value the game actually receives. If this is an error for the
    // game's P2P socket creation, the failure lives *inside our continuation*.
    LOG_INFO(Lib_Net, "depth={} result={:#x} entry_ret={:#x} entry_rsp={:#x}",
             g_sock_continuation_depth, static_cast<std::uint32_t>(result), entry_ret, entry_rsp);
    --g_sock_continuation_depth;
    return result;
}

HOOK_INIT(sceNetResolverStartNtoa);
s32 sceNetResolverStartNtoa_hook(s32 rid, const char* hostname, OrbisNetInAddr* addr, s32 timeout,
                                 s32 retry, s32 flags) {
    // v27: capture full 64-bit entry registers before anything else runs. The
    // typed parameters above truncate to 32 bits; the raw values distinguish
    // "junk argument" from "valid pointer whose low half looks small".
    std::uintptr_t r_rdi = 0, r_rsi = 0, r_rdx = 0, r_rcx = 0, r_r8 = 0, r_r9 = 0;
    std::uintptr_t r_rsp = 0, r_rbp = 0, r_rip = 0;
    asm volatile("movq %%rdi, %0" : "=r"(r_rdi));
    asm volatile("movq %%rsi, %0" : "=r"(r_rsi));
    asm volatile("movq %%rdx, %0" : "=r"(r_rdx));
    asm volatile("movq %%rcx, %0" : "=r"(r_rcx));
    asm volatile("movq %%r8, %0" : "=r"(r_r8));
    asm volatile("movq %%r9, %0" : "=r"(r_r9));
    asm volatile("movq %%rsp, %0" : "=r"(r_rsp));
    asm volatile("movq %%rbp, %0" : "=r"(r_rbp));
    asm volatile("lea 0(%%rip), %0" : "=r"(r_rip));
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
        // v27: full-width registers + stack anchors + whether a socket-call
        // continuation was on the stack when this fired.
        LOG_WARNING(Lib_Net,
                    "raw64 rdi={:#x} rsi={:#x} rdx={:#x} rcx={:#x} r8={:#x} r9={:#x} rsp={:#x} "
                    "rbp={:#x} rip={:#x} sockdepth={}",
                    r_rdi, r_rsi, r_rdx, r_rcx, r_r8, r_r9, r_rsp, r_rbp, r_rip,
                    g_sock_continuation_depth);
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
    ShadNetLogHookInstall("net.sceNetSocket", &Detour_sceNetSocket);
    HOOK(sceNetResolverStartNtoa);
    ShadNetLogHookInstall("net.sceNetResolverStartNtoa", &Detour_sceNetResolverStartNtoa);
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
