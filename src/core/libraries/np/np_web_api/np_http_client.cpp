// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#include "core/libraries/np/np_web_api/np_http_client.h"

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <orbis/Net.h>

#include "common/logging/log.h"

namespace Libraries::Np::NpWebApi {

namespace {

constexpr int kConnectTimeoutMs = 5000;
constexpr int kRecvTimeoutMs = 10000;
constexpr s32 kFail = -1;

struct TemplateRec {
    s32 lib_ctx = 0;
};

struct ConnectionRec {
    std::string url;
};

struct RequestRec {
    s32 conn_id = 0;
    std::string method;
    std::string url;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string resp_headers; // status line + headers, as received
    std::string resp_body;
    size_t read_pos = 0;
    s32 status = 0;
};

// Plain std::mutex: zero-fill construction, safe during the plugin's load-time
// initializer pass (no pthread calls at load - see client_main.cpp).
std::mutex g_mutex;
std::map<s32, TemplateRec> g_templates;
std::map<s32, ConnectionRec> g_connections;
std::map<s32, RequestRec> g_requests;
s32 g_next_id = 1;

std::string ToLower(const std::string& s) {
    std::string r = s;
    for (char& c : r)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return r;
}

bool ParseUrl(const std::string& url, std::string& host, u16& port, std::string& path) {
    constexpr const char* kScheme = "http://";
    if (url.rfind(kScheme, 0) != 0)
        return false;
    const size_t begin = std::strlen(kScheme);
    const size_t slash = url.find('/', begin);
    std::string hostport = url.substr(begin, slash == std::string::npos
                                                 ? std::string::npos
                                                 : slash - begin);
    path = (slash == std::string::npos) ? "/" : url.substr(slash);
    const size_t colon = hostport.rfind(':');
    if (colon != std::string::npos) {
        host = hostport.substr(0, colon);
        port = static_cast<u16>(std::atoi(hostport.c_str() + colon + 1));
    } else {
        host = hostport;
        port = 80;
    }
    return !host.empty() && port != 0;
}

bool ParseIpv4(const std::string& host, OrbisNetInAddr& out) {
    unsigned a = 0, b = 0, c = 0, d = 0;
    if (std::sscanf(host.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
        return false;
    if (a > 255 || b > 255 || c > 255 || d > 255)
        return false;
    out.s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
    return true;
}

bool WaitSocket(s32 sock, bool for_write, int timeout_ms) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    struct timeval tv{timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    const int rc = ::select(static_cast<int>(sock) + 1, for_write ? nullptr : &fds,
                            for_write ? &fds : nullptr, nullptr, &tv);
    return rc > 0;
}

s32 ConnectTcp(const std::string& host, u16 port, s32& out_sock) {
    OrbisNetInAddr addr{};
    if (!ParseIpv4(host, addr)) {
        // Only name-resolution-free targets are supported; our WebAPI server is
        // always configured as an IP literal.
        LOG_ERROR(Lib_NpWebApi, "NpHttp: host '{}' is not an IPv4 literal", host);
        return kFail;
    }

    const s32 sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        return kFail;

    s32 optval = 1;
    ::setsockopt(sock, SOL_SOCKET, SO_NBIO, &optval, sizeof(optval));

    OrbisNetSockaddrIn sin{};
    sin.sin_len = sizeof(OrbisNetSockaddrIn);
    sin.sin_family = AF_INET;
    sin.sin_addr = addr.s_addr;
    sin.sin_port = htons(port);
    int cr = ::connect(sock, reinterpret_cast<sockaddr*>(&sin), sizeof(sin));

    bool connected = (cr == 0);
    if (!connected && cr < 0 && errno == EINPROGRESS) {
        if (WaitSocket(sock, true, kConnectTimeoutMs)) {
            int err = 0;
            socklen_t len = sizeof(err);
            ::getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
            connected = (err == 0);
        }
    }
    if (!connected) {
        ::close(sock);
        LOG_ERROR(Lib_NpWebApi, "NpHttp: connect to {}:{} failed", host, port);
        return kFail;
    }

    optval = 0;
    ::setsockopt(sock, SOL_SOCKET, SO_NBIO, &optval, sizeof(optval));
    struct timeval rtv{kRecvTimeoutMs / 1000, (kRecvTimeoutMs % 1000) * 1000};
    ::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &rtv, sizeof(rtv));

    out_sock = sock;
    return 0;
}

bool SendAllRaw(s32 sock, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        const int r = static_cast<int>(
            ::send(sock, data.data() + sent, static_cast<int>(data.size() - sent), 0));
        if (r <= 0)
            return false;
        sent += static_cast<size_t>(r);
    }
    return true;
}

// Reads from the socket until we have at least `want` bytes (or the peer
// closes). Returns false on timeout/error before `want` bytes arrived.
bool RecvUntil(s32 sock, std::string& buffer, size_t want) {
    char tmp[4096];
    while (buffer.size() < want) {
        const int r = static_cast<int>(::recv(sock, tmp, sizeof(tmp), 0));
        if (r > 0) {
            buffer.append(tmp, static_cast<size_t>(r));
            continue;
        }
        if (r == 0)
            return false;
        if (errno == EINTR)
            continue;
        return false;
    }
    return true;
}

// Reads until the peer closes (used when there is no Content-Length).
void RecvToEnd(s32 sock, std::string& buffer) {
    char tmp[4096];
    for (;;) {
        const int r = static_cast<int>(::recv(sock, tmp, sizeof(tmp), 0));
        if (r > 0) {
            buffer.append(tmp, static_cast<size_t>(r));
            continue;
        }
        return;
    }
}

size_t DecodeChunked(const std::string& in, size_t pos, std::string& out) {
    while (pos < in.size()) {
        const size_t eol = in.find("\r\n", pos);
        if (eol == std::string::npos)
            break;
        const std::string size_str = in.substr(pos, eol - pos);
        const unsigned long chunk = std::strtoul(size_str.c_str(), nullptr, 16);
        pos = eol + 2;
        if (chunk == 0)
            break;
        if (pos + chunk > in.size())
            break;
        out.append(in, pos, chunk);
        pos += chunk + 2; // skip data + CRLF
    }
    return pos;
}

} // namespace

s32 NpHttpPerform(const std::string& method, const std::string& url,
                  const std::vector<std::pair<std::string, std::string>>& headers,
                  const void* body, u64 body_size, s32& out_status, std::string& out_headers,
                  std::string& out_body) {
    std::string host, path;
    u16 port = 0;
    if (!ParseUrl(url, host, port, path)) {
        LOG_ERROR(Lib_NpWebApi, "NpHttp: unsupported url '{}'", url);
        return kFail;
    }

    s32 sock = -1;
    if (ConnectTcp(host, port, sock) != 0)
        return kFail;

    std::string req;
    req.reserve(256 + (body_size > 0 ? body_size : 0));
    req += method;
    req += ' ';
    req += path;
    req += " HTTP/1.1\r\nHost: ";
    req += host;
    req += ':';
    req += std::to_string(port);
    req += "\r\nConnection: close\r\n";
    for (const auto& [name, value] : headers) {
        req += name;
        req += ": ";
        req += value;
        req += "\r\n";
    }
    if (body_size > 0) {
        req += "Content-Length: ";
        req += std::to_string(body_size);
        req += "\r\n";
    }
    req += "\r\n";
    if (body_size > 0) {
        req.append(reinterpret_cast<const char*>(body), static_cast<size_t>(body_size));
    }

    if (!SendAllRaw(sock, req)) {
        ::close(sock);
        LOG_ERROR(Lib_NpWebApi, "NpHttp: send failed for {} {}", method, url);
        return kFail;
    }

    std::string raw;
    // Cheap response framing: headers first, then the body per Content-Length
    // (or chunked, or read-until-close).
    for (;;) {
        const size_t header_end = raw.find("\r\n\r\n");
        if (header_end != std::string::npos)
            break;
        if (raw.size() > 64u * 1024u || !RecvUntil(sock, raw, raw.size() + 1))
            break;
    }
    const size_t header_end = raw.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        ::close(sock);
        LOG_ERROR(Lib_NpWebApi, "NpHttp: no response headers from {} {}", method, url);
        return kFail;
    }

    out_headers = raw.substr(0, header_end + 4);
    // Status code = first line, token #2.
    {
        const std::string first = out_headers.substr(0, out_headers.find("\r\n"));
        const size_t sp = first.find(' ');
        out_status = (sp == std::string::npos) ? 0 : std::atoi(first.c_str() + sp + 1);
    }

    const std::string header_block_lower = ToLower(out_headers);
    size_t body_start = header_end + 4;

    const size_t cl_pos = header_block_lower.find("content-length:");
    const bool chunked = header_block_lower.find("transfer-encoding:") != std::string::npos &&
                         header_block_lower.find("chunked") != std::string::npos;
    if (cl_pos != std::string::npos) {
        const size_t eol = out_headers.find("\r\n", cl_pos);
        const unsigned long want =
            std::strtoul(out_headers.c_str() + cl_pos + 15, nullptr, 10);
        if (!RecvUntil(sock, raw, body_start + want))
            LOG_WARNING(Lib_NpWebApi, "NpHttp: short body for {} {}", method, url);
        out_body = raw.substr(body_start, want <= raw.size() - body_start
                                               ? want
                                               : std::string::npos);
    } else if (chunked) {
        RecvToEnd(sock, raw);
        size_t pos = body_start;
        DecodeChunked(raw, pos, out_body);
    } else {
        RecvToEnd(sock, raw);
        if (raw.size() > body_start)
            out_body = raw.substr(body_start);
    }

    ::close(sock);
    LOG_INFO(Lib_NpWebApi, "NpHttp: {} {} -> {} ({} bytes)", method, url, out_status,
             out_body.size());
    return 0;
}

// ============================== shims ==============================

s32 NpHttpCreateTemplate(s32 lib_http_ctx_id, const char* /*user_agent*/, s32 /*http_ver*/,
                         s32 /*allow_caching*/) {
    std::lock_guard lock(g_mutex);
    const s32 id = g_next_id++;
    g_templates[id] = TemplateRec{lib_http_ctx_id};
    return id;
}

s32 NpHttpCreateConnectionWithURL(s32 tmpl_id, const char* url, bool /*enable_keepalive*/) {
    std::lock_guard lock(g_mutex);
    if (g_templates.find(tmpl_id) == g_templates.end())
        return kFail;
    const s32 id = g_next_id++;
    g_connections[id] = ConnectionRec{url != nullptr ? url : ""};
    return id;
}

s32 NpHttpCreateRequestWithURL(s32 conn_id, s32 method, const char* url, u64 /*content_length*/) {
    static const char* const kMethods[] = {
        "GET",  // 0
        "POST", // 1
        "GET",  // 2 (HEAD is unused by the layer)
        "GET",  // 3
        "PUT",  // 4
        "DELETE", // 5
        "GET",    // 6
        "GET",    // 7
        "PATCH",  // 8
    };
    if (method < 0 || method > 8)
        return kFail;
    std::lock_guard lock(g_mutex);
    if (g_connections.find(conn_id) == g_connections.end())
        return kFail;
    const s32 id = g_next_id++;
    RequestRec rec;
    rec.conn_id = conn_id;
    rec.method = kMethods[method];
    rec.url = url != nullptr ? url : "";
    g_requests[id] = std::move(rec);
    return id;
}

s32 NpHttpAddRequestHeader(s32 req_id, const char* name, const char* value, s32 /*overwrite*/) {
    std::lock_guard lock(g_mutex);
    auto it = g_requests.find(req_id);
    if (it == g_requests.end())
        return kFail;
    it->second.headers.emplace_back(name != nullptr ? name : "",
                                    value != nullptr ? value : "");
    return 0;
}

s32 NpHttpSendRequest(s32 req_id, const void* data, u64 size) {
    std::string method, url;
    std::vector<std::pair<std::string, std::string>> headers;
    {
        std::lock_guard lock(g_mutex);
        auto it = g_requests.find(req_id);
        if (it == g_requests.end())
            return kFail;
        method = it->second.method;
        url = it->second.url;
        headers = it->second.headers;
    }

    s32 status = 0;
    std::string resp_headers, resp_body;
    const s32 rc = NpHttpPerform(method, url, headers, data, size, status, resp_headers, resp_body);

    std::lock_guard lock(g_mutex);
    auto it = g_requests.find(req_id);
    if (it == g_requests.end())
        return kFail;
    if (rc != 0)
        return rc;
    it->second.status = status;
    it->second.resp_headers = std::move(resp_headers);
    it->second.resp_body = std::move(resp_body);
    it->second.read_pos = 0;
    return 0;
}

s32 NpHttpGetStatusCode(s32 req_id, s32* status) {
    std::lock_guard lock(g_mutex);
    auto it = g_requests.find(req_id);
    if (it == g_requests.end())
        return kFail;
    if (status != nullptr)
        *status = it->second.status;
    return 0;
}

s32 NpHttpReadData(s32 req_id, void* buf, u64 size) {
    std::lock_guard lock(g_mutex);
    auto it = g_requests.find(req_id);
    if (it == g_requests.end())
        return kFail;
    RequestRec& rec = it->second;
    const size_t remaining = rec.resp_body.size() - rec.read_pos;
    const size_t n = remaining < size ? remaining : static_cast<size_t>(size);
    if (n > 0) {
        std::memcpy(buf, rec.resp_body.data() + rec.read_pos, n);
        rec.read_pos += n;
    }
    return static_cast<s32>(n);
}

s32 NpHttpGetAllResponseHeaders(s32 req_id, char** buf, u64* size) {
    std::lock_guard lock(g_mutex);
    auto it = g_requests.find(req_id);
    if (it == g_requests.end())
        return kFail;
    // Points into the request record; valid until the request is deleted. The
    // caller only reads it immediately.
    if (buf != nullptr)
        *buf = const_cast<char*>(it->second.resp_headers.data());
    if (size != nullptr)
        *size = it->second.resp_headers.size();
    return 0;
}

s32 NpHttpAbortRequest(s32 /*req_id*/) {
    return 0;
}

s32 NpHttpDeleteRequest(s32 req_id) {
    std::lock_guard lock(g_mutex);
    g_requests.erase(req_id);
    return 0;
}

s32 NpHttpDeleteConnection(s32 conn_id) {
    std::lock_guard lock(g_mutex);
    g_connections.erase(conn_id);
    return 0;
}

s32 NpHttpDeleteTemplate(s32 tmpl_id) {
    std::lock_guard lock(g_mutex);
    g_templates.erase(tmpl_id);
    return 0;
}

} // namespace Libraries::Np::NpWebApi
