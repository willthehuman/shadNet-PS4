// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cerrno>
#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

#include "common/hook_diag.h"
#include "common/logging/log.h"
#include "core/libraries/kernel/kernel.h"

// See net.cpp: never pass a game-supplied C-string straight into fmt/strlen —
// short junk pointers (e.g. 0x2) crash on strlen. (CE-34878-0 fix class.)
static inline bool IsPlausibleString(const char* p) {
    return reinterpret_cast<std::uintptr_t>(p) >= 0x10000;
}

// Decode an AF_INET sockaddr into "a.b.c.d:port" for the socket-call trace.
// Address and port travel in network byte order — read the bytes directly.
static std::string DescribeSockaddr(const void* addr, u32 addrlen) {
    if (addr == nullptr || addrlen < 16) {
        return "<none>";
    }
    const auto* sa = reinterpret_cast<const sockaddr*>(addr);
    if (sa->sa_family != 2) {
        return fmt::format("family={}", sa->sa_family);
    }
    const auto* in4 = reinterpret_cast<const sockaddr_in*>(addr);
    const auto* b = reinterpret_cast<const u8*>(&in4->sin_addr.s_addr);
    const u16 port = static_cast<u16>((in4->sin_port >> 8) | (in4->sin_port << 8));
    return fmt::format("{}.{}.{}.{}:{}", b[0], b[1], b[2], b[3], port);
}

HOOK_INIT(socket);
s32 socket_hook(s32 family, s32 type, s32 protocol) {
    // The game's network layer (extnetwork2) creates type-6 P2P datagram
    // sockets ("...udpp2p..."), which on a jailbroken console land on the real
    // (dead-end) Sony P2P stack. Bridge them to plain UDP sockets: same
    // datagram semantics for LAN play, and the fd flows back through libSceNet
    // normally.
    s32 effective_type = type;
    if (family == 2 && type == 6) {
        effective_type = 2; // SOCK_DGRAM
    }
    const s32 rc = SHADNET_HOOK_CONTINUE(socket, family, effective_type, protocol);
    LOG_INFO(Lib_Kernel, "socket(family={}, type={}{}, protocol={}) -> {} [errno={}]", family, type,
             type != effective_type ? " -> bridged to DGRAM" : "", protocol, rc, errno);
    return rc;
}

HOOK_INIT(bind);
s32 bind_hook(s32 fd, const void* addr, u32 addrlen) {
    const s32 rc = SHADNET_HOOK_CONTINUE(bind, fd, reinterpret_cast<const sockaddr*>(addr),
                                         static_cast<socklen_t>(addrlen));
    LOG_INFO(Lib_Kernel, "bind(fd={}, addr={}) -> {} [errno={}]", fd,
             DescribeSockaddr(addr, addrlen), rc, errno);
    return rc;
}

HOOK_INIT(connect);
s32 connect_hook(s32 fd, const void* addr, u32 addrlen) {
    const s32 rc = SHADNET_HOOK_CONTINUE(connect, fd, reinterpret_cast<const sockaddr*>(addr),
                                         static_cast<socklen_t>(addrlen));
    LOG_INFO(Lib_Kernel, "connect(fd={}, addr={}) -> {} [errno={}]", fd,
             DescribeSockaddr(addr, addrlen), rc, errno);
    return rc;
}

// Sony P2P titles set P2P-specific socket options (SO_USECRYPTO / SO_USESIGNATURE / ...) on
// their network sockets. On plain/bridged UDP fds the real stack answers
// EINVAL/ENOPROTOOPT/EOPNOTSUPP — failures that abort the game's network setup and (observed
// behavior) lead to "fell back offline". The working PC port's policy is "never fail a socket
// option — accept unknown ones as no-op" (p2p_sockets.cpp SetSocketOptions). Soften ONLY the
// option-unsupported class; genuine errors (EBADF etc.) still pass through.
static bool OptionUnsupported(s32 rc) {
    if (rc >= 0) {
        return false;
    }
    switch (errno) {
    case EINVAL:
        return true;
#ifdef ENOPROTOOPT
    case ENOPROTOOPT:
        return true;
#endif
#ifdef EOPNOTSUPP
    case EOPNOTSUPP: // == ENOTSUP on FreeBSD; one entry covers both
        return true;
#endif
    default:
        return false;
    }
}

HOOK_INIT(setsockopt);
s32 setsockopt_hook(s32 fd, s32 level, s32 optname, const void* optval, u32 optlen) {
    const s32 rc = SHADNET_HOOK_CONTINUE(setsockopt, fd, level, optname,
                                         reinterpret_cast<const void*>(optval),
                                         static_cast<socklen_t>(optlen));
    const s32 out = OptionUnsupported(rc) ? 0 : rc;
    LOG_INFO(Lib_Kernel,
             "setsockopt(fd={}, level={}, opt={:#x}, len={}) -> {} (raw={}, errno={})", fd, level,
             optname, optlen, out, rc, errno);
    return out;
}

HOOK_INIT(getsockopt);
s32 getsockopt_hook(s32 fd, s32 level, s32 optname, void* optval, u32* optlen) {
    const s32 rc = SHADNET_HOOK_CONTINUE(getsockopt, fd, level, optname, optval,
                                         reinterpret_cast<socklen_t*>(optlen));
    const s32 out = OptionUnsupported(rc) ? 0 : rc;
    LOG_INFO(Lib_Kernel, "getsockopt(fd={}, level={}, opt={:#x}) -> {} (raw={}, errno={})", fd,
             level, optname, out, rc, errno);
    return out;
}

// Log-only observers: the game's P2P stack queries names of its sockets during setup.
// Pure observation — never alters the result.
HOOK_INIT(getsockname);
s32 getsockname_hook(s32 fd, void* addr, u32* addrlen) {
    const s32 rc = SHADNET_HOOK_CONTINUE(getsockname, fd, reinterpret_cast<sockaddr*>(addr),
                                         reinterpret_cast<socklen_t*>(addrlen));
    LOG_INFO(Lib_Kernel, "getsockname(fd={}) -> {} [errno={}]", fd, rc, errno);
    return rc;
}

HOOK_INIT(getpeername);
s32 getpeername_hook(s32 fd, void* addr, u32* addrlen) {
    const s32 rc = SHADNET_HOOK_CONTINUE(getpeername, fd, reinterpret_cast<sockaddr*>(addr),
                                         reinterpret_cast<socklen_t*>(addrlen));
    LOG_INFO(Lib_Kernel, "getpeername(fd={}) -> {} [errno={}]", fd, rc, errno);
    return rc;
}

HOOK_INIT(sendto);
s64 sendto_hook(s32 fd, const void* buf, u64 len, s32 flags, const void* addr, u32 addrlen) {
    const s64 rc = SHADNET_HOOK_CONTINUE(sendto, fd, buf, static_cast<size_t>(len), flags,
                                         reinterpret_cast<const sockaddr*>(addr),
                                         static_cast<socklen_t>(addrlen));
    if (rc < 0) {
        LOG_INFO(Lib_Kernel, "sendto(fd={}, addr={}, len={}) -> {} [errno={}]", fd,
                 DescribeSockaddr(addr, addrlen), len, rc, errno);
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
    // Same type-6 -> UDP bridge as socket_hook (libSceNet routes named
    // creations through here).
    s32 effective_type = type;
    if (family == 2 && type == 6) {
        effective_type = 2; // SOCK_DGRAM
    }
    s32 rc;
    if (IsPlausibleString(name)) {
        rc = SHADNET_HOOK_CONTINUE(__sys_socketex, name, family, effective_type, protocol);
        LOG_INFO(Lib_Kernel, "socketex(name={}, family={}, type={}{}) -> {}", name, family, type,
                 type != effective_type ? " -> bridged to DGRAM" : "", rc);
    } else {
        rc = SHADNET_HOOK_CONTINUE(__sys_socketex, name, family, effective_type, protocol);
        LOG_WARNING(Lib_Kernel,
                    "socketex(name=<bad ptr {:#x}>, family={}, type={}) -> {}",
                    reinterpret_cast<std::uintptr_t>(name), family, type, rc);
    }
    return rc;
}

void RegisterKernelHooks() {
    HOOK(socket);
    ShadNetLogHookInstall("kernel.socket", &Detour_socket);
    HOOK(__sys_socketex);
    ShadNetLogHookInstall("kernel.__sys_socketex", &Detour___sys_socketex);
    HOOK(bind);
    ShadNetLogHookInstall("kernel.bind", &Detour_bind);
    HOOK(connect);
    ShadNetLogHookInstall("kernel.connect", &Detour_connect);
    HOOK(setsockopt);
    ShadNetLogHookInstall("kernel.setsockopt", &Detour_setsockopt);
    HOOK(getsockopt);
    ShadNetLogHookInstall("kernel.getsockopt", &Detour_getsockopt);
    HOOK(getsockname);
    ShadNetLogHookInstall("kernel.getsockname", &Detour_getsockname);
    HOOK(getpeername);
    ShadNetLogHookInstall("kernel.getpeername", &Detour_getpeername);
    HOOK(sendto);
    ShadNetLogHookInstall("kernel.sendto", &Detour_sendto);
    HOOK(recvfrom);
    ShadNetLogHookInstall("kernel.recvfrom", &Detour_recvfrom);
}

namespace Libraries::Kernel::Kernel {

void RegisterHooks() {
    return RegisterKernelHooks();
}
} // namespace Libraries::Kernel::Kernel
