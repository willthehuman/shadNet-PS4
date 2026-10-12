// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Minimal real UDP transport for the NP signalling layer. One datagram socket
// carries the STUN echo exchange (client -> server :udp, server -> client
// pong) that the game's network-status check depends on. Everything goes
// through the same POSIX socket surface the shadNet client already uses
// (plain ::socket/::bind/::sendto/::recvfrom) - the raw SDK entry point
// rejects datagram sockets in this runtime (SCE 0x80410116 / EINVAL).
// Addresses are raw network-order sin_addr values and ports travel in the
// same byte order the NP port's callers pass them in.

#include "core/libraries/network/p2p_transport.h"

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/logging/log.h"

#ifndef SOCK_DGRAM_P2P
#define SOCK_DGRAM_P2P 6
#endif

namespace Libraries::Net {

namespace {

std::mutex g_mutex;
int g_sock = -1;
u16 g_port_net = 0;   // bound port, network order
u32 g_local_addr = 0; // LAN address, network order
u32 g_server_addr = 0;
u16 g_server_udp = 0; // server STUN port, network order

int CreateDatagramSocket() {
    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        const int first_err = errno;
        // Fall back to the P2P datagram type the signalling ioctl socket
        // already uses on this platform.
        sock = ::socket(AF_INET, SOCK_DGRAM_P2P, 0);
        LOG_INFO(Lib_Net, "P2P transport: SOCK_DGRAM failed (errno={}), SOCK_DGRAM_P2P -> {}",
                 first_err, sock);
    }
    return sock;
}

} // namespace

void P2PSetServerAddr(u32 server_addr, u16 server_udp_port) {
    std::scoped_lock lock(g_mutex);
    g_server_addr = server_addr;
    g_server_udp = server_udp_port;
}

bool EnsureP2PTransport() {
    std::scoped_lock lock(g_mutex);
    if (g_sock >= 0) {
        return true;
    }

    int sock = CreateDatagramSocket();
    if (sock < 0) {
        LOG_ERROR(Lib_Net, "P2P transport: socket() failed (errno={})", errno);
        return false;
    }

    const int flags = ::fcntl(sock, F_GETFL, 0);
    if (flags >= 0) {
        ::fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }

    sockaddr_in bind_addr{};
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = 0;
    bind_addr.sin_addr.s_addr = 0;
    if (::bind(sock, reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr)) < 0) {
        LOG_ERROR(Lib_Net, "P2P transport: bind() failed (errno={})", errno);
        ::close(sock);
        return false;
    }

    sockaddr_in local{};
    socklen_t local_len = sizeof(local);
    if (::getsockname(sock, reinterpret_cast<sockaddr*>(&local), &local_len) == 0) {
        g_port_net = local.sin_port;
    }

    // Advertised LAN address: ask the routing layer which local address it
    // would use to reach the shadNet server (datagram connect + getsockname).
    if (g_server_addr != 0) {
        const int probe = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (probe >= 0) {
            sockaddr_in target{};
            target.sin_family = AF_INET;
            target.sin_port = g_server_udp;
            target.sin_addr.s_addr = g_server_addr;
            if (::connect(probe, reinterpret_cast<sockaddr*>(&target), sizeof(target)) == 0) {
                sockaddr_in mine{};
                socklen_t mine_len = sizeof(mine);
                if (::getsockname(probe, reinterpret_cast<sockaddr*>(&mine), &mine_len) == 0) {
                    g_local_addr = mine.sin_addr.s_addr;
                }
            }
            ::close(probe);
        }
    }

    g_sock = sock;
    LOG_INFO(Lib_Net,
             "P2P transport: ready, fd={} port={} local={}.{}.{}.{} server={:#x}:{}", sock,
             ntohs(g_port_net), (g_local_addr >> 0) & 0xff, (g_local_addr >> 8) & 0xff,
             (g_local_addr >> 16) & 0xff, (g_local_addr >> 24) & 0xff, g_server_addr,
             ntohs(g_server_udp));
    return true;
}

bool P2PTransportIsReady() {
    return g_sock >= 0;
}

int P2PSignalingSendTo(const void* data, u32 len, u32 dest_addr, u16 dest_port) {
    if (!EnsureP2PTransport()) {
        return -1;
    }
    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = dest_port; // network order (callers pass wire values)
    dst.sin_addr.s_addr = dest_addr;
    const ssize_t rc =
        ::sendto(g_sock, data, len, 0, reinterpret_cast<const sockaddr*>(&dst), sizeof(dst));
    return static_cast<int>(rc);
}

int P2PSignalingRecvFrom(void* buf, u32 len, u32* from_addr, u16* from_port) {
    if (!EnsureP2PTransport()) {
        return -1;
    }
    sockaddr_in src{};
    socklen_t src_len = sizeof(src);
    const ssize_t rc = ::recvfrom(g_sock, buf, len, 0, reinterpret_cast<sockaddr*>(&src), &src_len);
    if (rc > 0) {
        if (from_addr != nullptr) {
            *from_addr = src.sin_addr.s_addr;
        }
        if (from_port != nullptr) {
            *from_port = src.sin_port;
        }
    }
    return static_cast<int>(rc);
}

int P2PControlSendTo(const void* data, u32 len, u32 dest_addr, u16 dest_port) {
    return P2PSignalingSendTo(data, len, dest_addr, dest_port);
}

int P2PControlRecvFrom(void* /*buf*/, u32 /*len*/, u32* /*from_addr*/, u16* /*from_port*/) {
    // Control traffic shares the datagram socket; draining it here could
    // consume signalling packets the receive thread needs, so report idle.
    return -1;
}

u16 GetP2PConfiguredPort() {
    return ntohs(g_port_net); // host order (matches the PC build's surface)
}

u32 GetP2PAdvertisedAddr() {
    return g_local_addr;
}

} // namespace Libraries::Net
