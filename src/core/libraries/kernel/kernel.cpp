// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdint>
#include <sys/socket.h>

#include "common/logging/log.h"
#include "core/libraries/kernel/kernel.h"

// See net.cpp: never pass a game-supplied C-string straight into fmt/strlen —
// short junk pointers (e.g. 0x2) crash on strlen. (CE-34878-0 fix class.)
static inline bool IsPlausibleString(const char* p) {
    return reinterpret_cast<std::uintptr_t>(p) >= 0x10000;
}

HOOK_INIT(socket);
s32 socket_hook(s32 family, s32 type, s32 protocol) {
    LOG_INFO(Lib_Kernel, "called, family = {}, type = {}, protocol = {}", family, type, protocol);
    return SHADNET_HOOK_CONTINUE(socket, family, type, protocol);
}

HOOK_INIT(__sys_socketex);
s32 __sys_socketex_hook(const char* name, s32 family, s32 type, s32 protocol) {
    if (IsPlausibleString(name)) {
        LOG_INFO(Lib_Kernel, "called, name = {}, family = {}, type = {}, protocol = {}", name,
                 family, type, protocol);
    } else {
        LOG_WARNING(Lib_Kernel,
                    "called, name = <bad ptr {:#x}>, family = {}, type = {}, protocol = {}",
                    reinterpret_cast<std::uintptr_t>(name), family, type, protocol);
    }
    return SHADNET_HOOK_CONTINUE(__sys_socketex, name, family, type, protocol);
}

void RegisterKernelHooks() {
    HOOK(socket);
    HOOK(__sys_socketex);
}

namespace Libraries::Kernel::Kernel {

void RegisterHooks() {
    return RegisterKernelHooks();
}
} // namespace Libraries::Kernel::Kernel
