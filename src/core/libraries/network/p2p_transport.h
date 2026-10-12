// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Minimal real UDP transport for the ported NP signalling layer (menu-stage
// and beyond): one datagram socket, bound to the console's LAN interface, used
// for the STUN echo exchange with the shadNet server and later for peer
// signalling traffic. Mirrors the surface of the shadps4 Net::P2P* helpers so
// the wiring site reads the same as the PC build.

#pragma once

#include "common/plugin_common.h"

namespace Libraries::Net {

// Announces the shadNet server endpoint (network byte order, matching the
// conventions used across the NP port). Called from SetMmShadNetClient.
void P2PSetServerAddr(u32 server_addr, u16 server_udp_port);

bool EnsureP2PTransport();
bool P2PTransportIsReady();
int P2PSignalingSendTo(const void* data, u32 len, u32 dest_addr, u16 dest_port);
int P2PSignalingRecvFrom(void* buf, u32 len, u32* from_addr, u16* from_port);
int P2PControlSendTo(const void* data, u32 len, u32 dest_addr, u16 dest_port);
int P2PControlRecvFrom(void* buf, u32 len, u32* from_addr, u16* from_port);
u16 GetP2PConfiguredPort();
u32 GetP2PAdvertisedAddr();

} // namespace Libraries::Net
