// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstring>
#include <deque>
#include <mutex>
#include <vector>
#include <absl/container/flat_hash_map.h>
#include <orbis/NpManager.h>
#include <orbis/libkernel.h>
#include "common/elf_info.h"
#include "common/logging/log.h"
#include "common/plugin_common.h"
#include "core/libraries/np/np_error.h"
#include "core/libraries/np/np_handler.h"
#include "core/libraries/np/np_manager/np_callbacks.h"
#include "core/libraries/np/np_manager/np_manager.h"

extern "C" {
// void sceNpRegisterStateCallbackForToolkit();
// void sceNpUnregisterStateCallbackForToolkit();

SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetState);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetNpId);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetOnlineId);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpCheckCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpCheckCallbackForLib);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpRegisterStateCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpRegisterStateCallbackA);
// SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpRegisterStateCallbackForToolkit);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpUnregisterStateCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpUnregisterStateCallbackA);
// SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpUnregisterStateCallbackForToolkit);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpCreateRequest);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpCreateAsyncRequest);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpAbortRequest);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpDeleteRequest);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpWaitAsync);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpPollAsync);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpSetNpTitleId);

void RegisterLibraryHooks() {
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetState);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetNpId);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetOnlineId);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpCheckCallback);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpCheckCallbackForLib);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpRegisterStateCallback);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpRegisterStateCallbackA);
    // SHADNET_HOOK(Libraries::Np::NpManager, sceNpRegisterStateCallbackForToolkit);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpUnregisterStateCallback);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpUnregisterStateCallbackA);
    // SHADNET_HOOK(Libraries::Np::NpManager, sceNpUnregisterStateCallbackForToolkit);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpCreateRequest);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpCreateAsyncRequest);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpAbortRequest);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpDeleteRequest);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpWaitAsync);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpPollAsync);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpSetNpTitleId);
}
}

namespace Libraries::Np::NpManager {

static s32 g_firmware_version = 0;

s32 sceNpGetState(s32 user_id, OrbisNpState* state) {
    if (user_id == -1 && g_firmware_version >= Common::ElfInfo::FW_900) {
        // FW < 9.00 behavior needs validating.
        LOG_ERROR(Lib_NpManager, "invalid user id {}", user_id);
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (!state) {
        LOG_ERROR(Lib_NpManager, "null state pointer");
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    
    if (NpHandler::Instance().IsSignedIn(user_id)) {
        LOG_INFO(Lib_NpManager, "called, returning signed in state");
        *state = ORBIS_NP_STATE_SIGNED_IN;
    } else {
        LOG_INFO(Lib_NpManager, "called, returning signed out state");
        *state = ORBIS_NP_STATE_SIGNED_OUT;
    }
    return ORBIS_OK;
}

s32 sceNpGetNpId(s32 user_id, OrbisNpId* np_id) {
    if (user_id == -1) {
        LOG_ERROR(Lib_NpManager, "invalid user id {}", user_id);
        return g_firmware_version >= Common::ElfInfo::FW_900 ? ORBIS_NP_ERROR_INVALID_ARGUMENT
                                                             : ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    if (!np_id) {
        LOG_ERROR(Lib_NpManager, "null np_id");
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (!NpHandler::Instance().IsSignedIn(user_id)) {
        // Not currently connected to shadNet, treat this as signed out.
        LOG_INFO(Lib_NpManager, "called, returning signed out");
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    LOG_INFO(Lib_NpManager, "called");
    *np_id = NpHandler::Instance().GetNpId(user_id);
    return ORBIS_OK;
}

s32 sceNpGetOnlineId(s32 user_id, OrbisNpOnlineId* online_id) {
    if (user_id == -1) {
        LOG_ERROR(Lib_NpManager, "invalid user id {}", user_id);
        return g_firmware_version >= Common::ElfInfo::FW_900 ? ORBIS_NP_ERROR_INVALID_ARGUMENT
                                                             : ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    if (!online_id) {
        LOG_ERROR(Lib_NpManager, "null online_id");
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (!NpHandler::Instance().IsSignedIn(user_id)) {
        // Not currently connected to shadNet, treat this as signed out.
        LOG_INFO(Lib_NpManager, "called, returning signed out");
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    LOG_INFO(Lib_NpManager, "called");
    *online_id = NpHandler::Instance().GetNpId(user_id).handle;
    return ORBIS_OK;
}

// ---- NP manager request table + title id (ported from the bloodborne_pc reference) ----

enum class NpRequestState {
    None = 0,
    Ready = 1,
    Aborted = 2,
    Complete = 3,
};

struct NpRequest {
    NpRequestState state;
    bool async;
    s32 result;
};

static std::vector<NpRequest> g_requests;
static s32 g_active_requests = 0;
static std::mutex g_request_mutex;

static s32 CreateNpRequest(bool async) {
    std::scoped_lock lk{g_request_mutex};

    if (g_active_requests == ORBIS_NP_MANAGER_REQUEST_LIMIT) {
        return ORBIS_NP_ERROR_REQUEST_MAX;
    }

    s32 req_index = 0;
    while (req_index < static_cast<s32>(g_requests.size())) {
        // Find first nonexistant request
        if (g_requests[req_index].state == NpRequestState::None) {
            g_requests[req_index].state = NpRequestState::Ready;
            g_requests[req_index].async = async;
            break;
        }
        req_index++;
    }

    if (req_index == static_cast<s32>(g_requests.size())) {
        NpRequest new_request{NpRequestState::Ready, async, 0};
        g_requests.emplace_back(new_request);
    }

    // Offset by one, first returned ID is 0x20000001
    g_active_requests++;
    return req_index + ORBIS_NP_MANAGER_REQUEST_ID_OFFSET + 1;
}

s32 sceNpCreateRequest() {
    LOG_DEBUG(Lib_NpManager, "called");
    return CreateNpRequest(false);
}

s32 sceNpCreateAsyncRequest(const OrbisNpCreateAsyncRequestParameter* param) {
    LOG_DEBUG(Lib_NpManager, "called");
    if (param == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (param->size != sizeof(OrbisNpCreateAsyncRequestParameter)) {
        return ORBIS_NP_ERROR_INVALID_SIZE;
    }
    return CreateNpRequest(true);
}

s32 sceNpAbortRequest(s32 req_id) {
    LOG_DEBUG(Lib_NpManager, "called req_id = {:#x}", req_id);

    std::scoped_lock lk{g_request_mutex};

    s32 req_index = req_id - ORBIS_NP_MANAGER_REQUEST_ID_OFFSET - 1;
    if (g_active_requests == 0 || req_index < 0 ||
        req_index >= static_cast<s32>(g_requests.size()) ||
        g_requests[req_index].state == NpRequestState::None) {
        return ORBIS_NP_ERROR_REQUEST_NOT_FOUND;
    }

    if (g_requests[req_index].state == NpRequestState::Complete) {
        // If the request is already complete, abort is ignored.
        return ORBIS_OK;
    }

    g_requests[req_index].state = NpRequestState::Aborted;
    return ORBIS_OK;
}

s32 sceNpDeleteRequest(s32 req_id) {
    LOG_DEBUG(Lib_NpManager, "called req_id = {:#x}", req_id);

    std::scoped_lock lk{g_request_mutex};

    s32 req_index = req_id - ORBIS_NP_MANAGER_REQUEST_ID_OFFSET - 1;
    if (g_active_requests == 0 || req_index < 0 ||
        req_index >= static_cast<s32>(g_requests.size()) ||
        g_requests[req_index].state == NpRequestState::None) {
        return ORBIS_NP_ERROR_REQUEST_NOT_FOUND;
    }

    g_active_requests--;
    g_requests[req_index].state = NpRequestState::None;
    return ORBIS_OK;
}

s32 sceNpWaitAsync(s32 req_id, s32* result) {
    if (result == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_request_mutex};

    s32 req_index = req_id - ORBIS_NP_MANAGER_REQUEST_ID_OFFSET - 1;
    if (g_active_requests == 0 || req_index < 0 ||
        req_index >= static_cast<s32>(g_requests.size()) ||
        g_requests[req_index].state == NpRequestState::None) {
        return ORBIS_NP_ERROR_REQUEST_NOT_FOUND;
    }

    if (!g_requests[req_index].async || g_requests[req_index].state == NpRequestState::Ready) {
        return ORBIS_NP_ERROR_INVALID_ID;
    }

    // Since we're not actually performing any sort of network request here,
    // we can just set result based on the request and return.
    *result = g_requests[req_index].result;
    LOG_WARNING(Lib_NpManager, "called req_id = {:#x}, returning result = {:#x}", req_id,
                static_cast<u32>(*result));
    return ORBIS_OK;
}

s32 sceNpPollAsync(s32 req_id, s32* result) {
    if (result == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_request_mutex};

    s32 req_index = req_id - ORBIS_NP_MANAGER_REQUEST_ID_OFFSET - 1;
    if (g_active_requests == 0 || req_index < 0 ||
        req_index >= static_cast<s32>(g_requests.size()) ||
        g_requests[req_index].state == NpRequestState::None) {
        return ORBIS_NP_ERROR_REQUEST_NOT_FOUND;
    }

    if (!g_requests[req_index].async || g_requests[req_index].state == NpRequestState::Ready) {
        return ORBIS_NP_ERROR_INVALID_ID;
    }

    // Since we're not actually performing any sort of network request here,
    // we can just set result based on the request and return.
    *result = g_requests[req_index].result;
    LOG_WARNING(Lib_NpManager, "called req_id = {:#x}, returning result = {:#x}", req_id,
                static_cast<u32>(*result));
    return ORBIS_OK;
}

s32 sceNpSetNpTitleId(OrbisNpTitleId* title_id, OrbisNpTitleSecret* title_secret) {
    if (title_id == nullptr || title_secret == nullptr) {
        LOG_ERROR(Lib_NpManager, "called with invalid arguments");
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    char id_buf[sizeof(title_id->id) + 1]{};
    std::memcpy(id_buf, title_id->id, sizeof(title_id->id));
    LOG_ERROR(Lib_NpManager, "(STUBBED) called, title_id = {}", id_buf);
    return ORBIS_OK;
}

void RegisterHooks() {
    sceKernelGetCompiledSdkVersion(&g_firmware_version);
    NpHandler::Instance().RegisterStateCallback(
        [](s32 user_id, OrbisNpState state) { QueueNpStateEvent(user_id, state); }, nullptr);

    return RegisterLibraryHooks();
}
} // namespace Libraries::Np::NpManager