// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#include "core/libraries/network/net_shim.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/logging/log.h"

namespace Libraries::Net {

s32 sceNetSocket(const char* name, s32 family, s32 type, s32 protocol) {
    const s32 sock = ::socket(family, type, protocol);
    LOG_DEBUG(Lib_Net, "net_shim: socket('{}', {}, {}, {}) -> {}", name ? name : "?", family, type,
              protocol, sock);
    return sock;
}

s32 sceNetSocketClose(s32 sock) {
    return ::close(sock);
}

s32 sceNetIoctl() {
    // shadps4 models this as a no-op probe; keep the same behaviour.
    return 0;
}

u16 sceNetNtohs(u16 value) {
    return ntohs(value);
}

} // namespace Libraries::Net
