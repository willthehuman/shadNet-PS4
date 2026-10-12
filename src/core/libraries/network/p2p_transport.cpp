// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Minimal real UDP transport for the NP signalling layer. One datagram socket
// carries the STUN echo exchange (client -> server :udp, server -> client
// pong) that the game's network-status check depends on. Addresses and ports
// keep the byte-order conventions already used across the NP port: u32
// addresses are the raw sin_addr values, ports are network order where the
// callers pass what they received.

#include "core/libraries/network/p2p_transport.h"

#include <mutex>

#include <orbis/Net.h>

#include "common/logging/log.h"

// PS4 sockopt constants (as defined by the toolchain's sys/socket.h).
#ifndef SOL_SOCKET
#define SOL_SOCKET 0xffff
#endif
#ifndef SO_NBIO
#define SO_NBIO 0x1200
#endif

namespace Libraries::Net {

namespace {

std::mutex g_mutex;
OrbisNetId g_sock = -1;
u16 g_port_net = 0;   // bound port, network order
u32 g_local_addr = 0; // LAN address, network order
u32 g_server_addr = 0;
u16 g_server_udp = 0; // server STUN port, network order

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

    OrbisNetId sock = sceNetSocket("shadNet-P2P", ORBIS_NET_AF_INET, ORBIS_NET_SOCK_DGRAM, 0);
    if (sock < 0) {
        LOG_ERROR(Lib_Net, "P2P transport: socket() failed ({:#x})", static_cast<u32>(sock));
        return false;
    }

    s32 nonblock = 1;
    const s32 nb_rc = sceNetSetsockopt(sock, SOL_SOCKET, SO_NBIO, &nonblock, sizeof(nonblock));
    if (nb_rc < 0) {
        LOG_WARNING(Lib_Net, "P2P transport: SO_NBIO failed ({:#x})", static_cast<u32>(nb_rc));
    }

    OrbisNetSockaddrIn bind_addr{};
    bind_addr.sin_len = sizeof(bind_addr);
    bind_addr.sin_family = ORBIS_NET_AF_INET;
    bind_addr.sin_port = 0;
    bind_addr.sin_addr = 0;
    const s32 bind_rc =
        sceNetBind(sock, reinterpret_cast<OrbisNetSockaddr*>(&bind_addr), sizeof(bind_addr));
    if (bind_rc < 0) {
        LOG_ERROR(Lib_Net, "P2P transport: bind() failed ({:#x})", static_cast<u32>(bind_rc));
        sceNetSocketClose(sock);
        return false;
    }

    OrbisNetSockaddrIn local{};
    OrbisNetSocklen_t local_len = sizeof(local);
    if (sceNetGetsockname(sock, reinterpret_cast<OrbisNetSockaddr*>(&local), &local_len) == 0) {
        g_port_net = local.sin_port;
    }

    // Advertised LAN address: ask the routing layer which local address it
    // would use to reach the shadNet server (datagram connect + getsockname).
    if (g_server_addr != 0) {
        OrbisNetId probe =
            sceNetSocket("shadNet-P2P-probe", ORBIS_NET_AF_INET, ORBIS_NET_SOCK_DGRAM, 0);
        if (probe >= 0) {
            OrbisNetSockaddrIn target{};
            target.sin_len = sizeof(target);
            target.sin_family = ORBIS_NET_AF_INET;
            target.sin_port = g_server_udp;
            target.sin_addr = g_server_addr;
            if (sceNetConnect(probe, reinterpret_cast<OrbisNetSockaddr*>(&target), sizeof(target)) ==
                0) {
                OrbisNetSockaddrIn mine{};
                OrbisNetSocklen_t mine_len = sizeof(mine);
                if (sceNetGetsockname(probe, reinterpret_cast<OrbisNetSockaddr*>(&mine),
                                      &mine_len) == 0) {
                    g_local_addr = mine.sin_addr;
                }
            }
            sceNetSocketClose(probe);
        }
    }

    g_sock = sock;
    LOG_INFO(Lib_Net, "P2P transport: ready, port={} local={:#x} server={:#x}:{}",
             sceNetNtohs(g_port_net), g_local_addr, g_server_addr, sceNetNtohs(g_server_udp));
    return true;
}

bool P2PTransportIsReady() {
    return g_sock >= 0;
}

int P2PSignalingSendTo(const void* data, u32 len, u32 dest_addr, u16 dest_port) {
    if (!EnsureP2PTransport()) {
        return -1;
    }
    OrbisNetSockaddrIn dst{};
    dst.sin_len = sizeof(dst);
    dst.sin_family = ORBIS_NET_AF_INET;
    dst.sin_port = dest_port; // network order (callers pass wire values)
    dst.sin_addr = dest_addr;
    const s32 rc = sceNetSendto(g_sock, data, len, 0,
                                reinterpret_cast<const OrbisNetSockaddr*>(&dst), sizeof(dst));
    return static_cast<int>(rc);
}

int P2PSignalingRecvFrom(void* buf, u32 len, u32* from_addr, u16* from_port) {
    if (!EnsureP2PTransport()) {
        return -1;
    }
    OrbisNetSockaddrIn src{};
    OrbisNetSocklen_t src_len = sizeof(src);
    const s32 rc = sceNetRecvfrom(g_sock, buf, len, 0,
                                  reinterpret_cast<OrbisNetSockaddr*>(&src), &src_len);
    if (rc > 0) {
        if (from_addr != nullptr) {
            *from_addr = src.sin_addr;
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
    return sceNetNtohs(g_port_net); // host order (matches the PC build's surface)
}

u32 GetP2PAdvertisedAddr() {
    return g_local_addr;
}

} // namespace Libraries::Net
