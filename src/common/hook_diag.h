// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

// v27 hook-integrity diagnostics (ps4-bloodborne-crossplay Task 4).
// Header-only so the explicit CMake source list needs no changes.
//
// The GoldHEN detour implementation copies the first `InstructionSize` bytes of
// the target function into a fresh mmap'd stub VERBATIM (no relocation of
// RIP-relative operands or relative branches) and appends an absolute jump back
// to target+InstructionSize. This dump captures everything needed to prove or
// disprove that hazard for each installed hook: the resolved target, the copied
// stub bytes, the trampoline bytes and the post-patch target bytes.
//
// Reading these addresses is safe: they are all pointers we own (or the mapped
// target function) inside our own process.

#pragma once

#include <Common.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "common/logging/log.h"

static inline void ShadNetFmtBytes(const void* p, std::uint32_t n, char* out,
                                   std::size_t out_len) {
    const auto* b = reinterpret_cast<const std::uint8_t*>(p);
    std::size_t k = 0;
    if (out_len == 0) {
        return;
    }
    out[0] = '\0';
    for (std::uint32_t i = 0; i < n && k + 3 < out_len; ++i) {
        std::snprintf(out + k, out_len - k, "%02x", b[i]);
        k += 2;
        if (i + 1 < n && k + 1 < out_len) {
            out[k++] = ' ';
        }
    }
    out[k] = '\0';
}

static inline std::uint32_t ShadNetFmtQwords(const std::uintptr_t* v, std::uint32_t n, char* out,
                                             std::size_t out_len) {
    std::size_t k = 0;
    if (out_len == 0) {
        return 0;
    }
    out[0] = '\0';
    std::uint32_t i = 0;
    for (; i < n && k + 20 < out_len; ++i) {
        std::snprintf(out + k, out_len - k, "%s%016llx", i == 0 ? "" : " ",
                      static_cast<unsigned long long>(v[i]));
        k += (i == 0 ? 16 : 17);
    }
    out[k] = '\0';
    return i;
}

static inline void ShadNetLogHookInstall(const char* name, const Detour* d);

// v28 FIX: relocate a leading relative jump in the copied stub prologue.
//
// The detour copies the target's first InstructionSize bytes into the stub
// verbatim. If the prologue begins with a relative jump (e.g. sceNetSocket's
// forwarder `e9 rel32`), replaying it from the stub jumps to
// `stub + 5 + rel` — garbage in the stub's own region — which (proven on
// 2026-10-12, round 23) walked the zero-fill into the *resolver trampoline's*
// page and hijacked the socket call into the resolver hook, returning EINVAL
// (0x80410116) to the game and breaking its P2P socket creation.
//
// Fix: rewrite the copied region as an ABSOLUTE jump (`ff 25 00 00 00 00
// <target>`, 14 bytes) to the same destination the original instruction
// targeted. StubSize is always InstructionSize + 14 with InstructionSize >= 14
// (SDK-enforced), so the region always fits exactly. The remaining copied
// bytes (dead code under the original unconditional jump) are NOP-filled.
static inline bool ShadNetFixupHookStub(const char* name, Detour* d) {
    if (d == nullptr || d->StubPtr == nullptr || d->StubSize < sizeof(d->JumpInstructions64)) {
        return false;
    }
    auto* stub = static_cast<std::uint8_t*>(d->StubPtr);
    const std::size_t copied = d->StubSize - sizeof(d->JumpInstructions64);
    if (copied < 14) {
        return false;
    }
    std::uintptr_t target = 0;
    bool have_target = false;
    if (stub[0] == 0xE9) { // jmp rel32
        std::int32_t rel = 0;
        std::memcpy(&rel, stub + 1, sizeof(rel));
        target = reinterpret_cast<std::uintptr_t>(d->FunctionPtr) + 5 +
                 static_cast<std::int64_t>(rel);
        have_target = true;
    } else if (stub[0] == 0xEB) { // jmp rel8
        const auto rel = static_cast<std::int8_t>(stub[1]);
        target = reinterpret_cast<std::uintptr_t>(d->FunctionPtr) + 2 +
                 static_cast<std::int64_t>(rel);
        have_target = true;
    }
    if (!have_target) {
        return false;
    }
    std::uint8_t abs_jmp[14] = {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0};
    std::memcpy(abs_jmp + 6, &target, sizeof(target));
    std::memcpy(stub, abs_jmp, sizeof(abs_jmp));
    std::memset(stub + sizeof(abs_jmp), 0x90, copied - sizeof(abs_jmp));
    LOG_WARNING(Lib_HookDiag,
                "hookfix[{}]: leading relative jump relocated in stub: orig={:#x} -> "
                "abs={:#x} (stub {:#x})",
                name, reinterpret_cast<std::uintptr_t>(d->FunctionPtr), target,
                reinterpret_cast<std::uintptr_t>(d->StubPtr));
    return true;
}

static inline void ShadNetLogHookInstall(const char* name, const Detour* d) {
    // v28: repair known-breakage prologue copies before dumping. Safe: only
    // rewrites a stub whose first instruction is an unconditional relative jmp.
    ShadNetFixupHookStub(name, const_cast<Detour*>(d));
    // The diag function's own address reveals the plugin's runtime base for
    // offline symbolization of every captured address.
    const auto diag_hint = reinterpret_cast<std::uintptr_t>(&ShadNetLogHookInstall);
    if (d == nullptr) {
        LOG_WARNING(Lib_HookDiag, "hookdiag[{}]: nullptr detour", name);
        return;
    }
    LOG_WARNING(Lib_HookDiag,
                "hookdiag[{}]: target={:#x} stub={:#x} stubsiz={} tramp={:#x} hook={:#x} "
                "diagbase={:#x}",
                name, reinterpret_cast<std::uintptr_t>(d->FunctionPtr),
                reinterpret_cast<std::uintptr_t>(d->StubPtr),
                static_cast<unsigned long long>(d->StubSize),
                reinterpret_cast<std::uintptr_t>(d->TrampolinePtr),
                reinterpret_cast<std::uintptr_t>(d->HookPtr), diag_hint);
    if (d->StubPtr != nullptr && d->StubSize != 0) {
        char hex[3 * 48 + 1]{};
        const auto n = static_cast<std::uint32_t>(d->StubSize < 48 ? d->StubSize : 48);
        ShadNetFmtBytes(d->StubPtr, n, hex, sizeof(hex));
        LOG_WARNING(Lib_HookDiag, "hookdiag[{}]: stub bytes: {}", name, hex);
    }
    if (d->FunctionPtr != nullptr) {
        char hex[3 * 24 + 1]{};
        ShadNetFmtBytes(d->FunctionPtr, 24, hex, sizeof(hex));
        LOG_WARNING(Lib_HookDiag, "hookdiag[{}]: target after patch: {}", name, hex);
    }
    if (d->TrampolinePtr != nullptr) {
        char hex[3 * 16 + 1]{};
        ShadNetFmtBytes(d->TrampolinePtr, 16, hex, sizeof(hex));
        LOG_WARNING(Lib_HookDiag, "hookdiag[{}]: trampoline bytes: {}", name, hex);
    }
}
