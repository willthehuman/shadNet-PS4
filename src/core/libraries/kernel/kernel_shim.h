// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "common/types.h"

// Plugin-side shims for the shadps4 kernel calls used by the ported NP
// modules. The signalling heap and munmap map onto regular aligned heap
// memory; nothing here needs real kernel memory semantics.
namespace Libraries::Kernel {

s32 sceKernelMapNamedFlexibleMemory(void** addr_out, u64 size, s32 prot, s32 flags,
                                    const char* name);
s32 sceKernelMunmap(void* addr, u64 size);
s32 sceKernelGetCompiledSdkVersion(s32* version);

} // namespace Libraries::Kernel
