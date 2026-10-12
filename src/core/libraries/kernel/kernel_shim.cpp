// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#include "core/libraries/kernel/kernel_shim.h"

#include <cstdlib>

#include "common/logging/log.h"

namespace Libraries::Kernel {

s32 sceKernelMapNamedFlexibleMemory(void** addr_out, u64 size, s32 /*prot*/, s32 /*flags*/,
                                    const char* name) {
    if (addr_out == nullptr || size == 0) {
        return -1;
    }
    // 16 KiB alignment mirrors the page granularity the callers assume.
    void* mem = std::aligned_alloc(0x4000, (size + 0x3fff) & ~0x3fffull);
    if (mem == nullptr) {
        LOG_ERROR(Lib_Kernel, "kernel_shim: aligned_alloc({}) failed for '{}'", size,
                  name ? name : "?");
        return -1;
    }
    *addr_out = mem;
    return 0;
}

s32 sceKernelMunmap(void* addr, u64 /*size*/) {
    std::free(addr);
    return 0;
}

s32 sceKernelGetCompiledSdkVersion(s32* version) {
    if (version == nullptr) {
        return -1;
    }
    // Report a modern firmware so version gates take their current paths.
    *version = 0x09030000;
    return 0;
}

} // namespace Libraries::Kernel
