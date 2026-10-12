// SPDX-License-Identifier: GPL-2.0-or-later
// Ported from the bloodborne_pc shadPS4 reference (Libraries::Np::NpAuth),
// adapted to the shadNet-PS4 plugin: the "shadnet enabled" gate is evaluated
// live from the NpHandler connection state instead of a startup setting.

#include <cstring>
#include <mutex>
#include <vector>

#include <orbis/NpAuth.h>
#include <orbis/UserService.h>

#include "common/logging/log.h"
#include "common/shadnet_hook.h"
#include "core/libraries/np/np_auth.h"
#include "core/libraries/np/np_error.h"
#include "core/libraries/np/np_handler.h"

// The SDK header declares most of the libSceNpAuth surface; fill the gaps.
extern "C" void sceNpAuthGetAuthorizationCodeV3();
extern "C" void sceNpAuthGetIdToken();
extern "C" void sceNpAuthGetIdTokenV3();

extern "C" {
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthCreateRequest);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthCreateAsyncRequest);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthGetAuthorizationCode);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthGetAuthorizationCodeA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthGetAuthorizationCodeV3);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthGetIdToken);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthGetIdTokenA);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthGetIdTokenV3);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthSetTimeout);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthAbortRequest);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthWaitAsync);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthPollAsync);
SHADNET_HOOK_DECLARE(Libraries::Np::NpAuth, sceNpAuthDeleteRequest);

void RegisterNpAuthHooks() {
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthCreateRequest);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthCreateAsyncRequest);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthGetAuthorizationCode);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthGetAuthorizationCodeA);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthGetAuthorizationCodeV3);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthGetIdToken);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthGetIdTokenA);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthGetIdTokenV3);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthSetTimeout);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthAbortRequest);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthWaitAsync);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthPollAsync);
    SHADNET_HOOK(Libraries::Np::NpAuth, sceNpAuthDeleteRequest);
}
}

namespace Libraries::Np::NpAuth {

static s32 g_active_auth_requests = 0;
static std::mutex g_auth_request_mutex;

// Internal types for storing request-related information
enum class NpAuthRequestState {
    None = 0,
    Ready = 1,
    Aborted = 2,
    Complete = 3,
};

struct NpAuthRequest {
    NpAuthRequestState state;
    bool async;
    s32 result;
};

static std::vector<NpAuthRequest> g_auth_requests;

// The reference gates this module behind a startup "shadnet enabled" setting.
// The plugin evaluates it live: ready once the handler is up and any user is
// signed in to shadNet.
static bool ShadNetReady() {
    auto& handler = NpHandler::Instance();
    return handler.IsActive() && handler.HasAnySignedInUser();
}

static s32 CreateNpAuthRequest(bool async) {
    if (g_active_auth_requests == ORBIS_NP_AUTH_REQUEST_LIMIT) {
        return ORBIS_NP_AUTH_ERROR_REQUEST_MAX;
    }

    std::scoped_lock lk{g_auth_request_mutex};

    s32 req_index = 0;
    while (req_index < static_cast<s32>(g_auth_requests.size())) {
        // Find first nonexistant request
        if (g_auth_requests[req_index].state == NpAuthRequestState::None) {
            // There is no request at this index, set the index to ready then break.
            g_auth_requests[req_index].state = NpAuthRequestState::Ready;
            g_auth_requests[req_index].async = async;
            break;
        }
        req_index++;
    }

    if (req_index == static_cast<s32>(g_auth_requests.size())) {
        // There are no requests to replace.
        NpAuthRequest new_request{NpAuthRequestState::Ready, async, 0};
        g_auth_requests.emplace_back(new_request);
    }

    // Offset by one, first returned ID is 0x10000001
    g_active_auth_requests++;
    LOG_DEBUG(Lib_NpAuth, "called, async = {}", async);
    return req_index + ORBIS_NP_AUTH_REQUEST_ID_OFFSET + 1;
}

s32 sceNpAuthCreateRequest() {
    return CreateNpAuthRequest(false);
}

s32 sceNpAuthCreateAsyncRequest(const OrbisNpAuthCreateAsyncRequestParameter* param) {
    if (param == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }
    if (param->size != sizeof(OrbisNpAuthCreateAsyncRequestParameter)) {
        return ORBIS_NP_AUTH_ERROR_INVALID_SIZE;
    }

    return CreateNpAuthRequest(true);
}

s32 GetAuthorizationCode(s32 req_id, const OrbisNpAuthGetAuthorizationCodeParameterA* param,
                         OrbisNpAuthorizationCode* auth_code, s32* issuer_id) {
    if (param == nullptr || auth_code == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }
    if (param->size != sizeof(OrbisNpAuthGetAuthorizationCodeParameter)) {
        return ORBIS_NP_AUTH_ERROR_INVALID_SIZE;
    }
    if (param->user_id == -1 || param->client_id == nullptr || param->scope == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_auth_request_mutex};

    // From here the actual authorization code request is performed.
    s32 req_index = req_id - ORBIS_NP_AUTH_REQUEST_ID_OFFSET - 1;
    if (g_active_auth_requests == 0 || static_cast<s32>(g_auth_requests.size()) <= req_index ||
        req_index < 0 || g_auth_requests[req_index].state == NpAuthRequestState::None) {
        return ORBIS_NP_AUTH_ERROR_REQUEST_NOT_FOUND;
    }

    auto& request = g_auth_requests[req_index];
    if (request.state == NpAuthRequestState::Complete) {
        request.result = ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    } else if (request.state == NpAuthRequestState::Aborted) {
        request.result = ORBIS_NP_AUTH_ERROR_ABORTED;
        return ORBIS_NP_AUTH_ERROR_ABORTED;
    }

    request.state = NpAuthRequestState::Complete;
    if (!ShadNetReady()) {
        request.result = ORBIS_NP_ERROR_SIGNED_OUT;
        // If the request is processed in some form, and it's an async request, then it returns OK.
        if (request.async) {
            return ORBIS_OK;
        }
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }

    LOG_ERROR(Lib_NpAuth, "(STUBBED) called, req_id = {:#x}, async = {}", req_id, request.async);

    // Not sure what values are expected here, so zeroing these for now.
    std::memset(auth_code, 0, sizeof(OrbisNpAuthorizationCode));
    std::strncpy(auth_code->code, "AUTHCODE", 9);
    if (issuer_id != nullptr) {
        *issuer_id = 100;
    }
    return ORBIS_OK;
}

s32 sceNpAuthGetAuthorizationCode(s32 req_id, const OrbisNpAuthGetAuthorizationCodeParameter* param,
                                  OrbisNpAuthorizationCode* auth_code, s32* issuer_id) {
    if (param == nullptr || auth_code == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }
    if (param->size != sizeof(OrbisNpAuthGetAuthorizationCodeParameter)) {
        return ORBIS_NP_AUTH_ERROR_INVALID_SIZE;
    }
    if (param->online_id == nullptr || param->client_id == nullptr || param->scope == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }
    if (!ShadNetReady()) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }

    // For simplicity, call sceUserServiceGetInitialUser to get the user_id.
    s32 user_id = 0;
    if (sceUserServiceGetInitialUser(&user_id) != 0) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }

    // Library constructs an OrbisNpAuthGetAuthorizationCodeParameterA struct,
    // then uses that to call GetAuthorizationCode.
    OrbisNpAuthGetAuthorizationCodeParameterA internal_params;
    std::memset(&internal_params, 0, sizeof(internal_params));
    internal_params.size = sizeof(internal_params);
    internal_params.client_id = param->client_id;
    internal_params.user_id = user_id;
    internal_params.scope = param->scope;

    return GetAuthorizationCode(req_id, &internal_params, auth_code, issuer_id);
}

s32 sceNpAuthGetAuthorizationCodeA(s32 req_id, const OrbisNpAuthGetAuthorizationCodeParameterA* param,
                                   OrbisNpAuthorizationCode* auth_code, s32* issuer_id) {
    return GetAuthorizationCode(req_id, param, auth_code, issuer_id);
}

s32 sceNpAuthGetAuthorizationCodeV3(s32 req_id, const OrbisNpAuthGetAuthorizationCodeParameterA* param,
                                    OrbisNpAuthorizationCode* auth_code, s32* issuer_id) {
    return GetAuthorizationCode(req_id, param, auth_code, issuer_id);
}

s32 GetIdToken(s32 req_id, const OrbisNpAuthGetIdTokenParameterA* param, OrbisNpIdToken* token) {
    if (param == nullptr || token == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }
    if (param->size != sizeof(OrbisNpAuthGetIdTokenParameterA)) {
        return ORBIS_NP_AUTH_ERROR_INVALID_SIZE;
    }
    if (param->user_id == -1 || param->client_id == nullptr || param->client_secret == nullptr ||
        param->scope == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_auth_request_mutex};

    // From here the actual id token request is performed.
    s32 req_index = req_id - ORBIS_NP_AUTH_REQUEST_ID_OFFSET - 1;
    if (g_active_auth_requests == 0 || static_cast<s32>(g_auth_requests.size()) <= req_index ||
        req_index < 0 || g_auth_requests[req_index].state == NpAuthRequestState::None) {
        return ORBIS_NP_AUTH_ERROR_REQUEST_NOT_FOUND;
    }

    auto& request = g_auth_requests[req_index];
    if (request.state == NpAuthRequestState::Complete) {
        request.result = ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    } else if (request.state == NpAuthRequestState::Aborted) {
        request.result = ORBIS_NP_AUTH_ERROR_ABORTED;
        return ORBIS_NP_AUTH_ERROR_ABORTED;
    }

    request.state = NpAuthRequestState::Complete;
    if (!ShadNetReady()) {
        request.result = ORBIS_NP_ERROR_SIGNED_OUT;
        // If the request is processed in some form, and it's an async request, then it returns OK.
        if (request.async) {
            return ORBIS_OK;
        }
        return ORBIS_NP_ERROR_SIGNED_OUT;
    }

    LOG_ERROR(Lib_NpAuth, "(STUBBED) called, req_id = {:#x}, async = {}", req_id, request.async);

    // Not sure what values are expected here, so zeroing this for now.
    std::memset(token, 0, sizeof(OrbisNpIdToken));
    std::strncpy(token->token, "TOKEN", 6);
    return ORBIS_OK;
}

s32 sceNpAuthGetIdToken(s32 req_id, const OrbisNpAuthGetIdTokenParameter* param, OrbisNpIdToken* token) {
    if (param == nullptr || token == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }
    if (param->size != sizeof(OrbisNpAuthGetIdTokenParameter)) {
        return ORBIS_NP_AUTH_ERROR_INVALID_SIZE;
    }
    if (param->online_id == nullptr || param->client_id == nullptr ||
        param->client_secret == nullptr || param->scope == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }
    if (!ShadNetReady()) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }

    // For simplicity, call sceUserServiceGetInitialUser to get the user_id.
    s32 user_id = 0;
    if (sceUserServiceGetInitialUser(&user_id) != 0) {
        return ORBIS_NP_ERROR_USER_NOT_FOUND;
    }

    // Library constructs an OrbisNpAuthGetIdTokenParameterA struct,
    // then uses that to call GetIdToken.
    OrbisNpAuthGetIdTokenParameterA internal_params;
    std::memset(&internal_params, 0, sizeof(internal_params));
    internal_params.size = sizeof(internal_params);
    internal_params.user_id = user_id;
    internal_params.client_id = param->client_id;
    internal_params.client_secret = param->client_secret;
    internal_params.scope = param->scope;

    return GetIdToken(req_id, &internal_params, token);
}

s32 sceNpAuthGetIdTokenA(s32 req_id, const OrbisNpAuthGetIdTokenParameterA* param, OrbisNpIdToken* token) {
    return GetIdToken(req_id, param, token);
}

s32 sceNpAuthGetIdTokenV3(s32 req_id, const OrbisNpAuthGetIdTokenParameterA* param, OrbisNpIdToken* token) {
    return GetIdToken(req_id, param, token);
}

s32 sceNpAuthSetTimeout(s32 req_id, s32 resolve_retry, u32 resolve_timeout, u32 conn_timeout,
                        u32 send_timeout, u32 recv_timeout) {
    LOG_ERROR(Lib_NpAuth, "(STUBBED) called");
    return ORBIS_OK;
}

s32 sceNpAuthAbortRequest(s32 req_id) {
    LOG_DEBUG(Lib_NpAuth, "called req_id = {:#x}", req_id);

    std::scoped_lock lk{g_auth_request_mutex};

    s32 req_index = req_id - ORBIS_NP_AUTH_REQUEST_ID_OFFSET - 1;
    if (g_active_auth_requests == 0 || static_cast<s32>(g_auth_requests.size()) <= req_index ||
        req_index < 0 || g_auth_requests[req_index].state == NpAuthRequestState::None) {
        return ORBIS_NP_AUTH_ERROR_REQUEST_NOT_FOUND;
    }

    if (g_auth_requests[req_index].state == NpAuthRequestState::Complete) {
        // If the request is already complete, abort is ignored.
        return ORBIS_OK;
    }

    g_auth_requests[req_index].state = NpAuthRequestState::Aborted;
    return ORBIS_OK;
}

s32 sceNpAuthWaitAsync(s32 req_id, s32* result) {
    if (result == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_auth_request_mutex};

    s32 req_index = req_id - ORBIS_NP_AUTH_REQUEST_ID_OFFSET - 1;
    if (g_active_auth_requests == 0 || static_cast<s32>(g_auth_requests.size()) <= req_index ||
        req_index < 0 || g_auth_requests[req_index].state == NpAuthRequestState::None) {
        return ORBIS_NP_AUTH_ERROR_REQUEST_NOT_FOUND;
    }

    if (!g_auth_requests[req_index].async ||
        g_auth_requests[req_index].state == NpAuthRequestState::Ready) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ID;
    }

    // Since we're not actually performing any sort of network request here,
    // we can just set result based on the request and return.
    *result = g_auth_requests[req_index].result;
    LOG_WARNING(Lib_NpAuth, "called req_id = {:#x}, returning result = {:#x}", req_id,
                static_cast<u32>(*result));
    return ORBIS_OK;
}

s32 sceNpAuthPollAsync(s32 req_id, s32* result) {
    if (result == nullptr) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ARGUMENT;
    }

    std::scoped_lock lk{g_auth_request_mutex};

    s32 req_index = req_id - ORBIS_NP_AUTH_REQUEST_ID_OFFSET - 1;
    if (g_active_auth_requests == 0 || static_cast<s32>(g_auth_requests.size()) <= req_index ||
        req_index < 0 || g_auth_requests[req_index].state == NpAuthRequestState::None) {
        return ORBIS_NP_AUTH_ERROR_REQUEST_NOT_FOUND;
    }

    if (!g_auth_requests[req_index].async ||
        g_auth_requests[req_index].state == NpAuthRequestState::Ready) {
        return ORBIS_NP_AUTH_ERROR_INVALID_ID;
    }

    // Since we're not actually performing any sort of network request here,
    // we can just set result based on the request and return.
    *result = g_auth_requests[req_index].result;
    LOG_WARNING(Lib_NpAuth, "called req_id = {:#x}, returning result = {:#x}", req_id,
                static_cast<u32>(*result));
    return ORBIS_OK;
}

s32 sceNpAuthDeleteRequest(s32 req_id) {
    LOG_DEBUG(Lib_NpAuth, "called req_id = {:#x}", req_id);

    std::scoped_lock lk{g_auth_request_mutex};

    s32 req_index = req_id - ORBIS_NP_AUTH_REQUEST_ID_OFFSET - 1;
    if (g_active_auth_requests == 0 || static_cast<s32>(g_auth_requests.size()) <= req_index ||
        req_index < 0 || g_auth_requests[req_index].state == NpAuthRequestState::None) {
        return ORBIS_NP_AUTH_ERROR_REQUEST_NOT_FOUND;
    }

    g_active_auth_requests--;
    g_auth_requests[req_index].state = NpAuthRequestState::None;
    return ORBIS_OK;
}

void RegisterHooks() {
    return RegisterNpAuthHooks();
}

} // namespace Libraries::Np::NpAuth
