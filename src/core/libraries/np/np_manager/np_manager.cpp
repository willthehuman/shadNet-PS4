// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <array>
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
// Not declared by the SDK's NpManager header; present in libSceNpManager exports.
s32 sceNpIsPlusMember(s32 user_id, bool* result);
s32 sceNpSetGamePresenceOnline(s32 req_id, const OrbisNpOnlineId* online_id, u32 presence);
s32 sceNpUnregisterGamePresenceCallbackA(s32 callback_id);

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
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpCheckNpAvailability);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpCheckNpAvailabilityA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpCheckNpReachability);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpCheckPlus);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetAccountCountry);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetAccountCountryA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetAccountDateOfBirth);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetAccountDateOfBirthA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetAccountId);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetAccountIdA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetAccountLanguage);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetAccountLanguageA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetGamePresenceStatus);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetGamePresenceStatusA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetNpReachabilityState);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetParentalControlInfo);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetParentalControlInfoA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetUserIdByAccountId);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpGetUserIdByOnlineId);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpHasSignedUp);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpIsPlusMember);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpSetContentRestriction);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpSetGamePresenceOnline);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpSetGamePresenceOnlineA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpSetTimeout);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpRegisterGamePresenceCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpRegisterGamePresenceCallbackA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpUnregisterGamePresenceCallbackA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpRegisterPlusEventCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpUnregisterPlusEventCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpRegisterNpReachabilityStateCallback);
SHADNET_HOOK_DECLARE(Libraries::Np::NpManager, sceNpUnregisterNpReachabilityStateCallback);

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
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpCheckNpAvailability);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpCheckNpAvailabilityA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpCheckNpReachability);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpCheckPlus);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetAccountCountry);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetAccountCountryA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetAccountDateOfBirth);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetAccountDateOfBirthA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetAccountId);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetAccountIdA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetAccountLanguage);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetAccountLanguageA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetGamePresenceStatus);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetGamePresenceStatusA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetNpReachabilityState);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetParentalControlInfo);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetParentalControlInfoA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetUserIdByAccountId);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpGetUserIdByOnlineId);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpHasSignedUp);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpIsPlusMember);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpSetContentRestriction);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpSetGamePresenceOnline);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpSetGamePresenceOnlineA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpSetTimeout);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpRegisterGamePresenceCallback);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpRegisterGamePresenceCallbackA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpUnregisterGamePresenceCallbackA);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpRegisterPlusEventCallback);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpUnregisterPlusEventCallback);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpRegisterNpReachabilityStateCallback);
    SHADNET_HOOK(Libraries::Np::NpManager, sceNpUnregisterNpReachabilityStateCallback);
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

// ---- Remaining NP manager surface (ported from the bloodborne_pc reference) ----

static bool ShadNetSignedIn(s32 user_id) {
    return Libraries::Np::NpHandler::Instance().IsSignedIn(user_id);
}

// Validate a request ID and return the NpRequest*, or nullptr + error code.
// Writes the error code to *out_err when returning nullptr.
static NpRequest* GetRequest(s32 req_id, s32* out_err) {
    if (req_id == 0) {
        *out_err = ORBIS_NP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    s32 req_index = req_id - ORBIS_NP_MANAGER_REQUEST_ID_OFFSET - 1;
    if (g_active_requests == 0 || req_index < 0 ||
        req_index >= static_cast<s32>(g_requests.size()) ||
        g_requests[req_index].state == NpRequestState::None) {
        *out_err = ORBIS_NP_ERROR_REQUEST_NOT_FOUND;
        return nullptr;
    }
    auto& req = g_requests[req_index];
    if (req.state == NpRequestState::Complete) {
        req.result = ORBIS_NP_ERROR_INVALID_ARGUMENT;
        *out_err = ORBIS_NP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    if (req.state == NpRequestState::Aborted) {
        req.result = ORBIS_NP_ERROR_ABORTED;
        *out_err = ORBIS_NP_ERROR_ABORTED;
        return nullptr;
    }
    return &req;
}

// Complete a request and return OK (or SIGNED_OUT for async when disabled).
static s32 CompleteRequest(NpRequest& req, s32 result) {
    req.state = NpRequestState::Complete;
    req.result = result;
    if (result != ORBIS_OK && req.async) {
        // Async requests always return OK immediately; result is read via PollAsync/WaitAsync.
        return ORBIS_OK;
    }
    return result;
}

// The plugin has no per-user profile system; serve region-neutral defaults
// (same defaults the reference uses when a profile is absent).
static void FillCountryCodeFromProfile(s32 /*user_id*/, OrbisNpCountryCode* out) {
    std::memset(out, 0, sizeof(OrbisNpCountryCode));
    out->data[0] = 'u';
    out->data[1] = 's';
}

static void FillLanguageCodeFromProfile(s32 /*user_id*/, OrbisNpLanguageCode* out) {
    std::memset(out, 0, sizeof(OrbisNpLanguageCode));
    out->code[0] = 'e';
    out->code[1] = 'n';
}

static s8 GetAgeFromProfile(s32 /*user_id*/) {
    return 13;
}

static void FillDateOfBirthFromProfile(s32 /*user_id*/, OrbisNpDate* out) {
    out->year = 2000;
    out->month = 1;
    out->day = 1;
}

s32 sceNpCheckNpAvailability(s32 req_id, OrbisNpOnlineId* online_id) {
    if (online_id == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 user_id = Libraries::Np::NpHandler::Instance().GetUserIdByOnlineId(*online_id);
    if (user_id == -1) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    return sceNpCheckNpAvailabilityA(req_id, user_id);
}

s32 sceNpCheckNpAvailabilityA(s32 req_id, s32 user_id) {
    if (user_id == -1) {
        LOG_ERROR(Lib_NpManager, "invalid user_id {}", user_id);
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    std::scoped_lock lk{g_request_mutex};
    s32 err;
    NpRequest* req = GetRequest(req_id, &err);
    if (!req) {
        return err;
    }
    if (!ShadNetSignedIn(user_id)) {
        return CompleteRequest(*req, ORBIS_NP_ERROR_SIGNED_OUT);
    }
    LOG_DEBUG(Lib_NpManager, "req_id = {:#x}, user_id = {}", req_id, user_id);
    return CompleteRequest(*req, ORBIS_OK);
}

s32 sceNpCheckNpReachability(s32 req_id, s32 user_id) {
    if (user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    std::scoped_lock lk{g_request_mutex};
    s32 err;
    NpRequest* req = GetRequest(req_id, &err);
    if (!req) {
        return err;
    }
    if (!ShadNetSignedIn(user_id)) {
        return CompleteRequest(*req, ORBIS_NP_ERROR_SIGNED_OUT);
    }
    LOG_DEBUG(Lib_NpManager, "req_id = {:#x}, user_id = {}", req_id, user_id);
    return CompleteRequest(*req, ORBIS_OK);
}

s32 sceNpCheckPlus(s32 req_id, const OrbisNpCheckPlusParameter* param,
                   OrbisNpCheckPlusResult* result) {
    if (req_id == 0 || param == nullptr || result == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (param->size != sizeof(OrbisNpCheckPlusParameter)) {
        return ORBIS_NP_ERROR_INVALID_SIZE;
    }
    if (param->user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (param->features < 1 || param->features > 3) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    // The reserved field must be zero-initialized by the caller.
    for (u8 b : param->reserved) {
        if (b != 0) {
            return ORBIS_NP_ERROR_INVALID_ARGUMENT;
        }
    }
    std::scoped_lock lk{g_request_mutex};
    s32 err;
    NpRequest* req = GetRequest(req_id, &err);
    if (!req) {
        return err;
    }
    if (!ShadNetSignedIn(param->user_id)) {
        return CompleteRequest(*req, ORBIS_NP_ERROR_SIGNED_OUT);
    }
    LOG_DEBUG(Lib_NpManager, "req_id = {:#x}, features = {:#x}", req_id, param->features);
    // Grant PS+ - shadNet has no subscription gating.
    result->authorized = true;
    return CompleteRequest(*req, ORBIS_OK);
}

s32 sceNpGetAccountCountry(OrbisNpOnlineId* online_id, OrbisNpCountryCode* country_code) {
    if (online_id == nullptr || country_code == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 user_id = Libraries::Np::NpHandler::Instance().GetUserIdByOnlineId(*online_id);
    if (user_id == -1) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    if (!ShadNetSignedIn(user_id)) {
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    FillCountryCodeFromProfile(user_id, country_code);
    return ORBIS_OK;
}

s32 sceNpGetAccountCountryA(s32 user_id, OrbisNpCountryCode* country_code) {
    if (country_code == nullptr || user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (!Libraries::Np::NpHandler::Instance().HasUser(user_id)) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    if (!ShadNetSignedIn(user_id)) {
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    FillCountryCodeFromProfile(user_id, country_code);
    return ORBIS_OK;
}

s32 sceNpGetAccountDateOfBirth(OrbisNpOnlineId* online_id, OrbisNpDate* date_of_birth) {
    if (online_id == nullptr || date_of_birth == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 user_id = Libraries::Np::NpHandler::Instance().GetUserIdByOnlineId(*online_id);
    if (user_id == -1) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    if (!ShadNetSignedIn(user_id)) {
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    FillDateOfBirthFromProfile(user_id, date_of_birth);
    return ORBIS_OK;
}

s32 sceNpGetAccountDateOfBirthA(s32 user_id, OrbisNpDate* date_of_birth) {
    if (date_of_birth == nullptr || user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (!Libraries::Np::NpHandler::Instance().HasUser(user_id)) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    if (!ShadNetSignedIn(user_id)) {
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    FillDateOfBirthFromProfile(user_id, date_of_birth);
    return ORBIS_OK;
}

s32 sceNpGetAccountId(OrbisNpOnlineId* online_id, u64* account_id) {
    LOG_DEBUG(Lib_NpManager, "called");
    if (online_id == nullptr || account_id == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 user_id = Libraries::Np::NpHandler::Instance().GetUserIdByOnlineId(*online_id);
    if (user_id == -1) {
        *account_id = 0;
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    if (!ShadNetSignedIn(user_id)) {
        *account_id = 0;
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    *account_id = Libraries::Np::NpHandler::Instance().GetAccountId(user_id);
    return ORBIS_OK;
}

s32 sceNpGetAccountIdA(s32 user_id, u64* account_id) {
    LOG_DEBUG(Lib_NpManager, "user_id {}", user_id);
    if (account_id == nullptr || user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (!ShadNetSignedIn(user_id)) {
        *account_id = 0;
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    *account_id = Libraries::Np::NpHandler::Instance().GetAccountId(user_id);
    return ORBIS_OK;
}

s32 sceNpGetAccountLanguage(s32 req_id, OrbisNpOnlineId* online_id, OrbisNpLanguageCode* language) {
    if (online_id == nullptr || language == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 user_id = Libraries::Np::NpHandler::Instance().GetUserIdByOnlineId(*online_id);
    if (user_id == -1) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    return Libraries::Np::NpManager::sceNpGetAccountLanguageA(req_id, user_id, language);
}

s32 sceNpGetAccountLanguageA(s32 /*req_id*/, s32 user_id, OrbisNpLanguageCode* language) {
    if (language == nullptr || user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (!ShadNetSignedIn(user_id)) {
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    FillLanguageCodeFromProfile(user_id, language);
    return ORBIS_OK;
}

s32 sceNpGetGamePresenceStatus(OrbisNpOnlineId* online_id, OrbisNpGamePresenseStatus* game_status) {
    if (online_id == nullptr || game_status == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 user_id = Libraries::Np::NpHandler::Instance().GetUserIdByOnlineId(*online_id);
    *game_status =
        (user_id != -1 && ShadNetSignedIn(user_id)) ? OrbisNpGamePresenseStatus::Online
                                                    : OrbisNpGamePresenseStatus::Offline;
    return ORBIS_OK;
}

s32 sceNpGetGamePresenceStatusA(s32 user_id, OrbisNpGamePresenseStatus* game_status) {
    if (game_status == nullptr || user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    *game_status = ShadNetSignedIn(user_id) ? OrbisNpGamePresenseStatus::Online
                                            : OrbisNpGamePresenseStatus::Offline;
    return ORBIS_OK;
}

s32 sceNpGetNpReachabilityState(s32 user_id, OrbisNpReachabilityState* state) {
    if (state == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    *state = ShadNetSignedIn(user_id) ? OrbisNpReachabilityState::Reachable
                                      : OrbisNpReachabilityState::Unavailable;
    return ORBIS_OK;
}

s32 sceNpGetParentalControlInfo(s32 req_id, OrbisNpOnlineId* online_id, s8* age,
                                OrbisNpParentalControlInfo* info) {
    if (online_id == nullptr || age == nullptr || info == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 user_id = Libraries::Np::NpHandler::Instance().GetUserIdByOnlineId(*online_id);
    if (user_id == -1) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    return sceNpGetParentalControlInfoA(req_id, user_id, age, info);
}

s32 sceNpGetParentalControlInfoA(s32 req_id, s32 user_id, s8* age, OrbisNpParentalControlInfo* info) {
    if (age == nullptr || info == nullptr || user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    std::scoped_lock lk{g_request_mutex};
    s32 err;
    NpRequest* req = GetRequest(req_id, &err);
    if (!req) {
        return err;
    }
    if (!ShadNetSignedIn(user_id)) {
        return CompleteRequest(*req, ORBIS_NP_ERROR_SIGNED_OUT);
    }
    LOG_DEBUG(Lib_NpManager, "req_id = {:#x}, user_id = {}", req_id, user_id);
    *age = GetAgeFromProfile(user_id);
    std::memset(info, 0, sizeof(OrbisNpParentalControlInfo));
    return CompleteRequest(*req, ORBIS_OK);
}

s32 sceNpGetUserIdByAccountId(u64 account_id, s32* user_id) {
    if (account_id == 0 || user_id == nullptr) {
        LOG_ERROR(Lib_NpManager, "invalid argument: account_id={}", account_id);
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 found = Libraries::Np::NpHandler::Instance().GetUserIdByAccountId(account_id);
    if (found == -1) {
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    // The resolved user must be a signed-in local user.
    if (!ShadNetSignedIn(found)) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    *user_id = found;
    LOG_DEBUG(Lib_NpManager, "account_id={} returns user_id={}", account_id, *user_id);
    return ORBIS_OK;
}

s32 sceNpGetUserIdByOnlineId(const OrbisNpOnlineId* online_id, s32* user_id) {
    if (online_id == nullptr || user_id == nullptr) {
        LOG_ERROR(Lib_NpManager, "invalid argument");
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const s32 found = Libraries::Np::NpHandler::Instance().GetUserIdByOnlineId(*online_id);
    if (found == -1) {
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }
    // The resolved user must be a signed-in local user.
    if (!ShadNetSignedIn(found)) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    *user_id = found;
    LOG_DEBUG(Lib_NpManager, "returns user_id={}", *user_id);
    return ORBIS_OK;
}

s32 sceNpHasSignedUp(s32 user_id, bool* has_signed_up) {
    LOG_DEBUG(Lib_NpManager, "called");
    if (has_signed_up == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    *has_signed_up = false;
    if (!Libraries::Np::NpHandler::Instance().HasUser(user_id)) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }
    // A user with a configured shadNet npid has "signed up". Independent of the
    // current connection state.
    *has_signed_up = true;
    return ORBIS_OK;
}

s32 sceNpIsPlusMember(s32 user_id, bool* result) {
    if (result == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    // shadNet has no PS Plus concept; report not a Plus member.
    *result = false;
    LOG_DEBUG(Lib_NpManager, "user_id {} not a Plus member", user_id);
    return ORBIS_OK;
}

s32 sceNpSetContentRestriction(const OrbisNpContentRestriction* restriction) {
    LOG_ERROR(Lib_NpManager, "(STUBBED) called");
    if (restriction == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (restriction->size != sizeof(OrbisNpContentRestriction)) {
        return ORBIS_NP_ERROR_INVALID_SIZE;
    }
    if (restriction->default_age_restriction < 0 || restriction->age_restriction_count > 0x100) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    if (restriction->age_restriction_count > 0 && restriction->age_restriction == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    return ORBIS_OK;
}

static std::mutex g_presence_plus_mutex;

static OrbisNpGamePresenceCallback g_game_presence_cb = nullptr;
static void* g_game_presence_cb_userdata = nullptr;

struct GamePresenceCallbackEntry {
    s32 handle;
    OrbisNpGamePresenceCallback func;
    void* userdata;
    bool in_use;
};
static std::array<GamePresenceCallbackEntry, 8> g_game_presence_cbs_a{};
static s32 g_game_presence_next_handle = 0;

static OrbisNpPlusEventCallback g_plus_event_cb = nullptr;
static void* g_plus_event_cb_userdata = nullptr;

s32 sceNpSetGamePresenceOnline(s32 req_id, const OrbisNpOnlineId* online_id, u32 presence) {
    if (req_id == 0 || online_id == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_request_mutex};
    s32 req_index = req_id - ORBIS_NP_MANAGER_REQUEST_ID_OFFSET - 1;
    if (g_active_requests == 0 || req_index < 0 ||
        req_index >= static_cast<s32>(g_requests.size()) ||
        g_requests[req_index].state == NpRequestState::None) {
        return ORBIS_NP_ERROR_REQUEST_NOT_FOUND;
    }

    // No live presence service under shadNet; accept and report success.
    LOG_DEBUG(Lib_NpManager, "(STUBBED) called req_id = {:#x}", req_id);
    return ORBIS_OK;
}

s32 sceNpSetGamePresenceOnlineA(s32 req_id, s32 user_id, u32 presence) {
    if (user_id == -1) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    const OrbisNpOnlineId online_id = Libraries::Np::NpHandler::Instance().GetOnlineId(user_id);
    return Libraries::Np::NpManager::sceNpSetGamePresenceOnline(req_id, &online_id, presence);
}

void sceNpRegisterGamePresenceCallback(OrbisNpGamePresenceCallback callback, void* userdata) {
    std::scoped_lock lk{g_presence_plus_mutex};
    g_game_presence_cb = callback;
    g_game_presence_cb_userdata = userdata;
}

s32 sceNpRegisterGamePresenceCallbackA(OrbisNpGamePresenceCallback callback, void* userdata) {
    if (callback == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_presence_plus_mutex};
    for (auto& entry : g_game_presence_cbs_a) {
        if (entry.in_use) {
            continue;
        }
        entry.handle = ++g_game_presence_next_handle;
        entry.func = callback;
        entry.userdata = userdata;
        entry.in_use = true;
        return entry.handle;
    }
    return ORBIS_NP_ERROR_CALLBACK_MAX;
}

s32 sceNpUnregisterGamePresenceCallbackA(s32 callback_id) {
    std::scoped_lock lk{g_presence_plus_mutex};
    for (auto& entry : g_game_presence_cbs_a) {
        if (entry.in_use && entry.handle == callback_id) {
            entry = {};
            return ORBIS_OK;
        }
    }
    return ORBIS_NP_ERROR_CALLBACK_NOT_REGISTERED;
}

s32 sceNpRegisterPlusEventCallback(OrbisNpPlusEventCallback callback, void* userdata) {
    if (callback == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    std::scoped_lock lk{g_presence_plus_mutex};
    if (g_plus_event_cb != nullptr) {
        return ORBIS_NP_ERROR_CALLBACK_ALREADY_REGISTERED;
    }
    g_plus_event_cb = callback;
    g_plus_event_cb_userdata = userdata;
    return ORBIS_OK;
}

s32 sceNpUnregisterPlusEventCallback() {
    std::scoped_lock lk{g_presence_plus_mutex};
    if (g_plus_event_cb == nullptr) {
        return ORBIS_NP_ERROR_CALLBACK_NOT_REGISTERED;
    }
    g_plus_event_cb = nullptr;
    g_plus_event_cb_userdata = nullptr;
    return ORBIS_OK;
}

s32 sceNpSetTimeout(s32 req_id, s32 resolve_retry, u32 resolve_timeout, u32 conn_timeout,
                    u32 send_timeout, u32 recv_timeout) {
    LOG_DEBUG(Lib_NpManager, "called req_id = {:#x}", req_id);

    if (req_id <= 0) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }

    // ORBIS_NP_TIMEOUT_NO_EFFECT (0) is allowed per field (use default) but not for every field.
    const auto unset_or_at_least = [](u32 v, u32 min_us) { return v == 0 || v >= min_us; };
    const bool all_zero = resolve_retry == 0 && resolve_timeout == 0 && conn_timeout == 0 &&
                          send_timeout == 0 && recv_timeout == 0;
    if (all_zero || resolve_retry < 0 || !unset_or_at_least(resolve_timeout, 1'000'000) ||
        !unset_or_at_least(conn_timeout, 10'000'000) ||
        !unset_or_at_least(send_timeout, 10'000'000) ||
        !unset_or_at_least(recv_timeout, 10'000'000)) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_request_mutex};
    s32 req_index = req_id - ORBIS_NP_MANAGER_REQUEST_ID_OFFSET - 1;
    if (g_active_requests == 0 || req_index < 0 ||
        req_index >= static_cast<s32>(g_requests.size()) ||
        g_requests[req_index].state == NpRequestState::None) {
        return ORBIS_NP_ERROR_REQUEST_NOT_FOUND;
    }

    // shadNet performs no real network I/O, so the timeouts are validated and accepted but
    // not applied.
    return ORBIS_OK;
}

void RegisterHooks() {
    sceKernelGetCompiledSdkVersion(&g_firmware_version);
    NpHandler::Instance().RegisterStateCallback(
        [](s32 user_id, OrbisNpState state) { QueueNpStateEvent(user_id, state); }, nullptr);

    return RegisterLibraryHooks();
}
} // namespace Libraries::Np::NpManager