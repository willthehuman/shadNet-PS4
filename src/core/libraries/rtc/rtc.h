// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

// Minimal RTC types needed by the ported Np type headers.

#pragma once

#include "common/types.h"

namespace Libraries::Rtc {

struct OrbisRtcTick {
    u64 tick;
};

} // namespace Libraries::Rtc
