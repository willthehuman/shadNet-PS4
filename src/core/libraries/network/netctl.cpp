// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#include "core/libraries/network/netctl.h"

#include <cstring>

#include "common/logging/log.h"

namespace NetCtl {

s32 sceNetCtlGetNatInfo(OrbisNetCtlNatInfo* nat_info) {
    if (nat_info == nullptr) {
        return -1;
    }
    std::memset(nat_info, 0, sizeof(*nat_info));
    nat_info->size = sizeof(*nat_info);
    // Type 1 = open NAT; signaling treats mapped_addr == 0 as "fall back to
    // the local address", which is what we want on a LAN session.
    nat_info->nat_type = 1;
    nat_info->mapped_addr = 0;
    LOG_DEBUG(Lib_Net, "netctl_shim: NAT info requested -> open NAT");
    return 0;
}

} // namespace NetCtl
