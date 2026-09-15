// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <orbis/Net.h>
#include "common/types.h"

namespace Libraries::Network::Net {

s32 sceNetSocket(const char* name, s32 family, s32 type, s32 protocol);
s32 sceNetResolverStartNtoa(s32 rid, const char* hostname, OrbisNetInAddr* addr, s32 timeout,
                            s32 retry, s32 flags);
void RegisterHooks();
} // namespace Libraries::Network::Net