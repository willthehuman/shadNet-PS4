// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cerrno>
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
    const s32 rc = SHADNET_HOOK_CONTINUE(socket, family, type, protocol);
    LOG_INFO(Lib_Kernel, "socket(family={}, type={}, protocol={}) -> {} [errno={}]", family, type,
             protocol, rc, errno);
    return rc;
}

HOOK_INIT(bind);
s32 bind_hook(s32 fd, const void* addr, u32 addrlen) {
    const s32 rc = SHADNET_HOOK_CONTINUE(bind, fd, reinterpret_cast<const sockaddr*>(addr),
                                         static_cast<socklen_t>(addrlen));
    LOG_INFO(Lib_Kernel, "bind(fd={}, addrlen={}) -> {} [errno={}]", fd, addrlen, rc, errno);
    return rc;
}

HOOK_INIT(connect);
s32 connect_hook(s32 fd, const void* addr, u32 addrlen) {
    const s32 rc = SHADNET_HOOK_CONTINUE(connect, fd, reinterpret_cast<const sockaddr*>(addr),
                                         static_cast<socklen_t>(addrlen));
    LOG_INFO(Lib_Kernel, "connect(fd={}, addrlen={}) -> {} [errno={}]", fd, addrlen, rc, errno);
    return rc;
}

HOOK_INIT(sendto);
s64 sendto_hook(s32 fd, const void* buf, u64 len, s32 flags, const void* addr, u32 addrlen) {
    const s64 rc = SHADNET_HOOK_CONTINUE(sendto, fd, buf, static_cast<size_t>(len), flags,
                                         reinterpret_cast<const sockaddr*>(addr),
                                         static_cast<socklen_t>(addrlen));
    if (rc < 0) {
        LOG_INFO(Lib_Kernel, "sendto(fd={}, len={}) -> {} [errno={}]", fd, len, rc, errno);
    }
    return rc;
}

HOOK_INIT(recvfrom);
s64 recvfrom_hook(s32 fd, void* buf, u64 len, s32 flags, void* addr, u32* addrlen) {
    const s64 rc = SHADNET_HOOK_CONTINUE(recvfrom, fd, buf, static_cast<size_t>(len), flags,
                                         reinterpret_cast<sockaddr*>(addr),
                                         reinterpret_cast<socklen_t*>(addrlen));
    if (rc > 0) {
        LOG_INFO(Lib_Kernel, "recvfrom(fd={}, len={}) -> {}", fd, len, rc);
    }
    return rc;
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
    HOOK(bind);
    HOOK(connect);
    HOOK(sendto);
    HOOK(recvfrom);
}

namespace Libraries::Kernel::Kernel {

void RegisterHooks() {
    return RegisterKernelHooks();
}
} // namespace Libraries::Kernel::Kernel
