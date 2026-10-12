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

static inline void ShadNetLogHookInstall(const char* name, const Detour* d) {
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
