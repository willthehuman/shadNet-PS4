// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

// v30 — HTTP/SSL observability hooks (log-only).
//
// The game's own HTTP stack (libSceHttp/libSceSsl) is the last major
// un-instrumented network surface: everything we DO serve looks healthy, yet
// the online state never engages. The working PC build answers the game's own
// server calls in its emulator HTTP layer (host_overrides.json); on console
// those calls — if any — are invisible to us today. These hooks answer the
// question "does the game actually issue HTTP(s) requests, and against which
// URLs" without changing any behavior.
//
// Design: ABI-transparent raw shims. Every hook receives the eight integer
// argument registers verbatim, logs them, and forwards the same values through
// the detour stub. No signature mismatch is possible (x86-64 SysV: 6 register +
// up to 2 stack args survive the round trip in order; all of these APIs take
// integers/pointers only). Return values pass through unchanged.

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "common/hook_diag.h"
#include "common/logging/log.h"
#include "common/plugin_common.h"
#include "core/libraries/network/http_hooks.h"

extern "C" {
// Own declarations: the OpenOrbis Http.h prototypes are not guaranteed to match
// the on-console ABI, and a mismatch would corrupt the forwarded arguments.
// These are only used to (a) resolve the import at link time and (b) name the
// detour; calls go through the detour stub with the caller's original values.
s64 sceHttpInit(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpTerm(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpCreateTemplate(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpDeleteTemplate(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpCreateConnectionWithURL(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpDeleteConnection(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpCreateRequestWithURL(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpDeleteRequest(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpSendRequest(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpAbortRequest(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpWaitRequest(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpReadData(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpGetStatusCode(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpGetResponseContentLength(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpAddRequestHeader(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpSetConnectTimeOut(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpSetNonblock(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceSslInit(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceSslTerm(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpsEnableOption(s64, s64, s64, s64, s64, s64, s64, s64);
s64 sceHttpsDisableOption(s64, s64, s64, s64, s64, s64, s64, s64);
}

// String probes are strictly limited to ranges that are mapped by definition
// (eboot image and the low library window) so a wild pointer can never fault.
static bool InProbeRange(s64 p) {
    const auto v = static_cast<u64>(p);
    return (v >= 0x400000ull && v < 0x5AD4000ull) ||   // eboot text+data
           (v >= 0x800000000ull && v < 0x801000000ull); // libkernel..early libs
}

static void LogHttpCall(const char* fn, s64 a1, s64 a2, s64 a3, s64 a4, s64 a5, s64 a6, s64 a7,
                        s64 a8, int s1, int s2) {
    LOG_INFO(Lib_HttpHook,
             "httphook: {} a1={:#x} a2={:#x} a3={:#x} a4={:#x} a5={:#x} a6={:#x} a7={:#x} a8={:#x}",
             fn, static_cast<u64>(a1), static_cast<u64>(a2), static_cast<u64>(a3),
             static_cast<u64>(a4), static_cast<u64>(a5), static_cast<u64>(a6), static_cast<u64>(a7),
             static_cast<u64>(a8));
    const s64 args[8] = {a1, a2, a3, a4, a5, a6, a7, a8};
    const int idxs[2] = {s1, s2};
    for (int idx : idxs) {
        if (idx < 1 || idx > 8) {
            continue;
        }
        const s64 p = args[idx - 1];
        if (!InProbeRange(p)) {
            continue;
        }
        const char* c = reinterpret_cast<const char*>(p);
        char buf[193]{};
        int n = 0;
        for (; n < 192; ++n) {
            const char ch = c[n];
            if (ch == '\0') {
                break;
            }
            if (ch < 0x20 || ch > 0x7e) {
                n = -1;
                break;
            }
        }
        if (n > 0) {
            std::memcpy(buf, c, static_cast<std::size_t>(n));
            LOG_INFO(Lib_HttpHook, "httphook: {} arg{} = \"{}\"", fn, idx, buf);
        }
    }
}

#define HTTP_TRACE_HOOK(name, s1, s2)                                                              \
    HOOK_INIT(name);                                                                               \
    static u32 name##_count = 0;                                                                   \
    static s64 name##_hook(s64 a1, s64 a2, s64 a3, s64 a4, s64 a5, s64 a6, s64 a7, s64 a8) {       \
        const u32 n = ++name##_count;                                                              \
        /* v31: the game pumps some of these (sceHttpWaitRequest ~3000x/session, likely per */     \
        /* frame); full logging there cost ~2 slow console-log writes per pump and tanked fps. */  \
        const bool verbose = (n <= 32) || ((n & 0x7F) == 0);                                       \
        if (verbose) {                                                                             \
            LogHttpCall(#name, a1, a2, a3, a4, a5, a6, a7, a8, s1, s2);                            \
        }                                                                                          \
        const s64 rc = SHADNET_HOOK_CONTINUE(name, a1, a2, a3, a4, a5, a6, a7, a8);                \
        if (verbose) {                                                                             \
            LOG_INFO(Lib_HttpHook, "httphook: {} [n={}] -> {:#x}", #name, n,                       \
                     static_cast<u64>(rc));                                                        \
        } else if ((n & 0x3FF) == 0) {                                                             \
            LOG_INFO(Lib_HttpHook, "httphook: {} [n={}] last rc={:#x} (rate-limited)", #name, n,   \
                     static_cast<u64>(rc));                                                        \
        }                                                                                          \
        return rc;                                                                                 \
    }

HTTP_TRACE_HOOK(sceHttpInit, 0, 0)
HTTP_TRACE_HOOK(sceHttpTerm, 0, 0)
HTTP_TRACE_HOOK(sceHttpCreateTemplate, 2, 0)              // arg2 = userAgent
HTTP_TRACE_HOOK(sceHttpDeleteTemplate, 0, 0)
HTTP_TRACE_HOOK(sceHttpCreateConnectionWithURL, 2, 0)     // arg2 = url
HTTP_TRACE_HOOK(sceHttpDeleteConnection, 0, 0)
HTTP_TRACE_HOOK(sceHttpCreateRequestWithURL, 3, 0)        // arg3 = url
HTTP_TRACE_HOOK(sceHttpDeleteRequest, 0, 0)
HTTP_TRACE_HOOK(sceHttpSendRequest, 0, 0)
HTTP_TRACE_HOOK(sceHttpAbortRequest, 0, 0)
HTTP_TRACE_HOOK(sceHttpWaitRequest, 0, 0)
HTTP_TRACE_HOOK(sceHttpReadData, 0, 0)
HTTP_TRACE_HOOK(sceHttpGetStatusCode, 0, 0)
HTTP_TRACE_HOOK(sceHttpGetResponseContentLength, 0, 0)
HTTP_TRACE_HOOK(sceHttpAddRequestHeader, 2, 3)            // arg2/arg3 = name/value
HTTP_TRACE_HOOK(sceHttpSetConnectTimeOut, 0, 0)
HTTP_TRACE_HOOK(sceHttpSetNonblock, 0, 0)
HTTP_TRACE_HOOK(sceSslInit, 0, 0)
HTTP_TRACE_HOOK(sceSslTerm, 0, 0)
HTTP_TRACE_HOOK(sceHttpsEnableOption, 0, 0)
HTTP_TRACE_HOOK(sceHttpsDisableOption, 0, 0)

#define HTTP_INSTALL_HOOK(name)                                                                    \
    HOOK(name);                                                                                    \
    ShadNetLogHookInstall("http." #name, &Detour_##name)

namespace Libraries::Network::Http {

void RegisterHooks() {
    HTTP_INSTALL_HOOK(sceHttpInit);
    HTTP_INSTALL_HOOK(sceHttpTerm);
    HTTP_INSTALL_HOOK(sceHttpCreateTemplate);
    HTTP_INSTALL_HOOK(sceHttpDeleteTemplate);
    HTTP_INSTALL_HOOK(sceHttpCreateConnectionWithURL);
    HTTP_INSTALL_HOOK(sceHttpDeleteConnection);
    HTTP_INSTALL_HOOK(sceHttpCreateRequestWithURL);
    HTTP_INSTALL_HOOK(sceHttpDeleteRequest);
    HTTP_INSTALL_HOOK(sceHttpSendRequest);
    HTTP_INSTALL_HOOK(sceHttpAbortRequest);
    HTTP_INSTALL_HOOK(sceHttpWaitRequest);
    HTTP_INSTALL_HOOK(sceHttpReadData);
    HTTP_INSTALL_HOOK(sceHttpGetStatusCode);
    HTTP_INSTALL_HOOK(sceHttpGetResponseContentLength);
    HTTP_INSTALL_HOOK(sceHttpAddRequestHeader);
    HTTP_INSTALL_HOOK(sceHttpSetConnectTimeOut);
    HTTP_INSTALL_HOOK(sceHttpSetNonblock);
    HTTP_INSTALL_HOOK(sceSslInit);
    HTTP_INSTALL_HOOK(sceSslTerm);
    HTTP_INSTALL_HOOK(sceHttpsEnableOption);
    HTTP_INSTALL_HOOK(sceHttpsDisableOption);
}

} // namespace Libraries::Network::Http
