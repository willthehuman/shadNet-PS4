// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Game-facing netctl: report the network as fully up. The working PC build
// implements libSceNetCtl as "wired link, IP obtained, everything fine"; on
// this jailbroken console the system netctl service has its own isolated view
// and the game's online check needs the plain picture. Values mirror the PC
// build (net_ctl_codes.h constants + sane per-code info fills).

#include <cstdio>
#include <cstring>

#include <orbis/NetCtl.h>

#include "common/logging/log.h"
#include "common/types.h"
#include "core/libraries/network/netctl_hooks.h"
#include "core/libraries/network/p2p_transport.h"

// Not declared by the trimmed OpenOrbis header; PS4 SDK signatures.
extern "C" {
int32_t sceNetCtlGetState(int32_t* state);
int32_t sceNetCtlRegisterCallback(void* callback, void* arg, int32_t* cid);
int32_t sceNetCtlUnregisterCallback(int32_t cid);
int32_t sceNetCtlCheckCallback();
int32_t sceNetCtlGetResult(int32_t event_type, int32_t* error_code);
}

namespace {

// net_ctl_codes.h (from the working PC build)
constexpr s32 kStateDisconnected = 0;
constexpr s32 kStateIpObtained = 3;
constexpr s32 kDeviceWired = 0;
constexpr s32 kLinkConnected = 1;
constexpr s32 kErrorInvalidAddr = static_cast<s32>(0x80412107);

void FillIpAddress(char* out, size_t out_size) {
    const u32 addr = Libraries::Net::GetP2PAdvertisedAddr();
    if (addr == 0) {
        std::strncpy(out, "192.168.50.90", out_size - 1);
        return;
    }
    std::snprintf(out, out_size, "%u.%u.%u.%u", (addr >> 0) & 0xff, (addr >> 8) & 0xff,
                  (addr >> 16) & 0xff, (addr >> 24) & 0xff);
}

} // namespace

HOOK_INIT(sceNetCtlInit);
s32 sceNetCtlInit_hook() {
    LOG_INFO(Lib_Net, "netctl: sceNetCtlInit -> ok");
    return 0;
}

HOOK_INIT(sceNetCtlGetState);
s32 sceNetCtlGetState_hook(s32* state) {
    if (state != nullptr) {
        *state = kStateIpObtained;
    }
    LOG_INFO(Lib_Net, "netctl: sceNetCtlGetState -> IPOBTAINED");
    return 0;
}

HOOK_INIT(sceNetCtlGetInfo);
s32 sceNetCtlGetInfo_hook(s32 code, OrbisNetCtlInfo* info) {
    if (info == nullptr) {
        return kErrorInvalidAddr;
    }
    LOG_INFO(Lib_Net, "netctl: sceNetCtlGetInfo code={}", code);
    std::memset(info, 0, sizeof(*info));
    switch (code) {
    case ORBIS_NET_CTL_INFO_DEVICE:
        info->device = kDeviceWired;
        break;
    case ORBIS_NET_CTL_INFO_ETHER_ADDR: {
        // Stable locally-administered address.
        const u8 mac[6] = {0x02, 0x00, 0x5A, 0x32, 0xA8, 0xC0};
        std::memcpy(info->ether_addr, mac, sizeof(mac));
        break;
    }
    case ORBIS_NET_CTL_INFO_MTU:
        info->mtu = 1500;
        break;
    case ORBIS_NET_CTL_INFO_LINK:
        info->link = kLinkConnected;
        break;
    case ORBIS_NET_CTL_INFO_IP_CONFIG:
        info->ip_config = 1; // static
        break;
    case ORBIS_NET_CTL_INFO_IP_ADDRESS:
        FillIpAddress(info->ip_address, sizeof(info->ip_address));
        break;
    case ORBIS_NET_CTL_INFO_NETMASK:
        std::strncpy(info->netmask, "255.255.255.0", sizeof(info->netmask) - 1);
        break;
    case ORBIS_NET_CTL_INFO_DEFAULT_ROUTE:
        std::strncpy(info->default_route, "192.168.50.1", sizeof(info->default_route) - 1);
        break;
    case ORBIS_NET_CTL_INFO_PRIMARY_DNS:
        std::strncpy(info->primary_dns, "192.168.50.1", sizeof(info->primary_dns) - 1);
        break;
    case ORBIS_NET_CTL_INFO_SECONDARY_DNS:
        std::strncpy(info->secondary_dns, "1.1.1.1", sizeof(info->secondary_dns) - 1);
        break;
    case ORBIS_NET_CTL_INFO_HTTP_PROXY_CONFIG:
        info->http_proxy_config = 0; // off
        break;
    case ORBIS_NET_CTL_INFO_HTTP_PROXY_PORT:
        info->http_proxy_port = 0;
        break;
    default:
        LOG_INFO(Lib_Net, "netctl: sceNetCtlGetInfo code={} -> empty", code);
        break;
    }
    return 0;
}

HOOK_INIT(sceNetCtlRegisterCallback);
s32 sceNetCtlRegisterCallback_hook(void* callback, void* arg, s32* cid) {
    LOG_INFO(Lib_Net, "netctl: sceNetCtlRegisterCallback cb={:p} arg={:p}", callback, arg);
    if (cid != nullptr) {
        *cid = 1;
    }
    return 0;
}

HOOK_INIT(sceNetCtlUnregisterCallback);
s32 sceNetCtlUnregisterCallback_hook(s32 cid) {
    LOG_INFO(Lib_Net, "netctl: sceNetCtlUnregisterCallback cid={}", cid);
    return 0;
}

HOOK_INIT(sceNetCtlCheckCallback);
s32 sceNetCtlCheckCallback_hook() {
    return 0;
}

HOOK_INIT(sceNetCtlGetResult);
s32 sceNetCtlGetResult_hook(s32 event_type, s32* error_code) {
    LOG_INFO(Lib_Net, "netctl: sceNetCtlGetResult event={}", event_type);
    if (error_code != nullptr) {
        *error_code = 0;
    }
    return 0;
}

static void RegisterNetCtlHooks() {
    HOOK(sceNetCtlInit);
    HOOK(sceNetCtlGetState);
    HOOK(sceNetCtlGetInfo);
    HOOK(sceNetCtlRegisterCallback);
    HOOK(sceNetCtlUnregisterCallback);
    HOOK(sceNetCtlCheckCallback);
    HOOK(sceNetCtlGetResult);
}

namespace Libraries::Network::NetCtl {

void RegisterHooks() {
    RegisterNetCtlHooks();
}

} // namespace Libraries::Network::NetCtl
