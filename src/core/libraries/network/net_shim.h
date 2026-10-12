// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "common/types.h"

// Plugin-side equivalents of the shadps4 net helpers the ported NP modules use.
// Plain sockets via the same POSIX surface the shadNet client already uses.
namespace Libraries::Net {

s32 sceNetSocket(const char* name, s32 family, s32 type, s32 protocol);
s32 sceNetSocketClose(s32 sock);
s32 sceNetIoctl();
u16 sceNetNtohs(u16 value);

} // namespace Libraries::Net
