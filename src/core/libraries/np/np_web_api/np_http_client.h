// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <string>
#include <utility>
#include <vector>
#include "common/types.h"

namespace Libraries::Np::NpWebApi {

// The NP webapi layer must talk to our own shadNet WebAPI server. The
// console's libSceHttp cannot get there: its internal resolver calls end up
// with junk hostname pointers and fail with EINVAL. This small blocking
// HTTP/1.1 client drives the requests directly over the plugin's own sockets
// (same POSIX calls the shadNet client uses) and exposes drop-in shims for
// the sceHttp* functions the ported webapi code calls.

// Perform one HTTP request against a plain-http URL ("http://host:port/path").
// Returns 0 on success (out_status/out_headers/out_body filled) or a negative
// error value on failure.
s32 NpHttpPerform(const std::string& method, const std::string& url,
                  const std::vector<std::pair<std::string, std::string>>& headers,
                  const void* body, u64 body_size, s32& out_status, std::string& out_headers,
                  std::string& out_body);

// -------- drop-in shims for the sceHttp* call shapes used by the layer --------
s32 NpHttpCreateTemplate(s32 lib_http_ctx_id, const char* user_agent, s32 http_ver,
                         s32 allow_caching);
s32 NpHttpCreateConnectionWithURL(s32 tmpl_id, const char* url, bool enable_keepalive);
s32 NpHttpCreateRequestWithURL(s32 conn_id, s32 method, const char* url, u64 content_length);
s32 NpHttpAddRequestHeader(s32 req_id, const char* name, const char* value, s32 overwrite);
s32 NpHttpSendRequest(s32 req_id, const void* data, u64 size);
s32 NpHttpGetStatusCode(s32 req_id, s32* status);
s32 NpHttpReadData(s32 req_id, void* buf, u64 size);
s32 NpHttpGetAllResponseHeaders(s32 req_id, char** buf, u64* size);
s32 NpHttpAbortRequest(s32 req_id);
s32 NpHttpDeleteRequest(s32 req_id);
s32 NpHttpDeleteConnection(s32 conn_id);
s32 NpHttpDeleteTemplate(s32 tmpl_id);

} // namespace Libraries::Np::NpWebApi
