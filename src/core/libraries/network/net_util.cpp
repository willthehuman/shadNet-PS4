// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#include "core/libraries/network/net_util.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <vector>

#include "common/logging/log.h"
#include "shadnet/config.h"

namespace NetUtil {

void NetUtilInternal::DetectLocalIp() const {
    ip_detected = true;
    ip = "0.0.0.0";

    // Determine the address of the interface that reaches our shadNet server:
    // open a UDP socket, "connect" it (no packets sent) and read back the
    // local address the kernel picked.
    std::string host = "192.168.50.233";
    u16 port = 31313;
    {
        auto& config = ShadNet::Settings::GetInstance();
        const std::string server = config.GetServerUrl(); // "host:port"
        const size_t colon = server.rfind(':');
        if (colon != std::string::npos) {
            host = server.substr(0, colon);
            port = static_cast<u16>(std::atoi(server.c_str() + colon + 1));
        } else if (!server.empty()) {
            host = server;
        }
    }

    const s32 sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return;
    }

    struct sockaddr_in dst {};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &dst.sin_addr) == 1) {
        if (::connect(sock, reinterpret_cast<struct sockaddr*>(&dst), sizeof(dst)) == 0) {
            struct sockaddr_in local {};
            socklen_t len = sizeof(local);
            if (::getsockname(sock, reinterpret_cast<struct sockaddr*>(&local), &len) == 0) {
                char buf[INET_ADDRSTRLEN] = {};
                if (inet_ntop(AF_INET, &local.sin_addr, buf, sizeof(buf)) != nullptr) {
                    ip = buf;
                }
            }
        }
    }
    ::close(sock);
    LOG_INFO(Lib_Net, "net_util: local address detected as {}", ip);
}

const std::string& NetUtilInternal::GetIp() const {
    std::lock_guard lock(m_mutex);
    if (!ip_detected) {
        DetectLocalIp();
    }
    return ip;
}

u32 NetUtilInternal::GetExternalIp() const {
    std::lock_guard lock(m_mutex);
    return external_ip;
}

void NetUtilInternal::SetExternalIp(u32 addr) {
    std::lock_guard lock(m_mutex);
    external_ip = addr;
}

u32 NetUtilInternal::GetNatType() const {
    std::lock_guard lock(m_mutex);
    return nat_type;
}

} // namespace NetUtil
