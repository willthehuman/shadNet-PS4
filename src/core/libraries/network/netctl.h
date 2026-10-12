// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "common/types.h"

// Minimal NetCtl surface for the signalling layer: only NAT info is consumed.
namespace NetCtl {

struct OrbisNetCtlNatInfo {
    u32 size;
    u32 nat_type;
    u32 mapped_addr;
    u8 reserved[4];
};

s32 sceNetCtlGetNatInfo(OrbisNetCtlNatInfo* nat_info);

} // namespace NetCtl
