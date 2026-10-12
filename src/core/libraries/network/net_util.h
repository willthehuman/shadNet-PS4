// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <mutex>
#include <string>

#include "common/types.h"

// Plugin-side NetUtil surface for the ported NP modules. Only the accessors
// the signalling layer consumes are present.
namespace NetUtil {

class NetUtilInternal {
public:
    explicit NetUtilInternal() = default;
    ~NetUtilInternal() = default;

    NetUtilInternal(const NetUtilInternal&) = delete;
    NetUtilInternal& operator=(const NetUtilInternal&) = delete;

    const std::string& GetIp() const;
    u32 GetExternalIp() const;
    void SetExternalIp(u32 addr);
    u32 GetNatType() const;

private:
    void DetectLocalIp() const;

    // Plain std::mutex members: zero-fill construction, safe to build during
    // the plugin's load-time initializer pass (no pthread calls at load).
    mutable std::mutex m_mutex;
    mutable std::string ip;
    mutable bool ip_detected = false;
    u32 external_ip = 0;
    u32 nat_type = 1;
};

} // namespace NetUtil
