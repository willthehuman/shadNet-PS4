// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <orbis/NpManager.h>
#include "common/types.h"
#include "core/libraries/np/np_manager/np_callbacks.h"

namespace Libraries::Np::NpManager {

constexpr s32 ORBIS_NP_MANAGER_REQUEST_LIMIT = 0x20;
constexpr s32 ORBIS_NP_MANAGER_REQUEST_ID_OFFSET = 0x20000000;

struct OrbisNpCreateAsyncRequestParameter {
    u64 size;
    u64 cpu_affinity_mask;
    s32 thread_priority;
    u8 padding[4];
};

enum class OrbisNpGamePresenseStatus {
    Offline = 0,
    Online = 1,
};

struct OrbisNpAgeRestriction {
    OrbisNpCountryCode country_code;
    s8 age;
    u8 padding[3];
};

struct OrbisNpContentRestriction {
    u64 size;
    s8 default_age_restriction;
    u8 padding[3];
    s32 age_restriction_count;
    const OrbisNpAgeRestriction* age_restriction;
};

struct OrbisNpCheckPlusParameter {
    u64 size;
    s32 user_id;
    u8 padding[4];
    u64 features;
    u8 reserved[32];
};

struct OrbisNpCheckPlusResult {
    bool authorized;
    u8 reserved[32];
};

// shadNet has no live game-presence or PS Plus event source, so registered callbacks are
// stored but never invoked.
using OrbisNpGamePresenceCallback = void (*)(const OrbisNpOnlineId* online_id, void* userdata);
using OrbisNpPlusEventCallback = void (*)(s32 user_id, s32 event, void* userdata);

s32 sceNpGetState(s32 user_id, OrbisNpState* state);
s32 sceNpGetNpId(s32 user_id, OrbisNpId* np_id);
s32 sceNpGetOnlineId(s32 user_id, OrbisNpOnlineId* online_id);
s32 sceNpCreateRequest();
s32 sceNpCreateAsyncRequest(const OrbisNpCreateAsyncRequestParameter* param);
s32 sceNpAbortRequest(s32 req_id);
s32 sceNpDeleteRequest(s32 req_id);
s32 sceNpWaitAsync(s32 req_id, s32* result);
s32 sceNpPollAsync(s32 req_id, s32* result);
s32 sceNpSetNpTitleId(OrbisNpTitleId* title_id, OrbisNpTitleSecret* title_secret);

s32 sceNpCheckNpAvailability(s32 req_id, OrbisNpOnlineId* online_id);
s32 sceNpCheckNpAvailabilityA(s32 req_id, s32 user_id);
s32 sceNpCheckNpReachability(s32 req_id, s32 user_id);
s32 sceNpCheckPlus(s32 req_id, const OrbisNpCheckPlusParameter* param,
                   OrbisNpCheckPlusResult* result);
s32 sceNpGetAccountCountry(OrbisNpOnlineId* online_id, OrbisNpCountryCode* country_code);
s32 sceNpGetAccountCountryA(s32 user_id, OrbisNpCountryCode* country_code);
s32 sceNpGetAccountDateOfBirth(OrbisNpOnlineId* online_id, OrbisNpDate* date_of_birth);
s32 sceNpGetAccountDateOfBirthA(s32 user_id, OrbisNpDate* date_of_birth);
s32 sceNpGetAccountId(OrbisNpOnlineId* online_id, u64* account_id);
s32 sceNpGetAccountIdA(s32 user_id, u64* account_id);
s32 sceNpGetAccountLanguage(s32 req_id, OrbisNpOnlineId* online_id, OrbisNpLanguageCode* language);
s32 sceNpGetAccountLanguageA(s32 req_id, s32 user_id, OrbisNpLanguageCode* language);
s32 sceNpGetGamePresenceStatus(OrbisNpOnlineId* online_id, OrbisNpGamePresenseStatus* game_status);
s32 sceNpGetGamePresenceStatusA(s32 user_id, OrbisNpGamePresenseStatus* game_status);
s32 sceNpGetNpReachabilityState(s32 user_id, OrbisNpReachabilityState* state);
s32 sceNpGetParentalControlInfo(s32 req_id, OrbisNpOnlineId* online_id, s8* age,
                                OrbisNpParentalControlInfo* info);
s32 sceNpGetParentalControlInfoA(s32 req_id, s32 user_id, s8* age,
                                 OrbisNpParentalControlInfo* info);
s32 sceNpGetUserIdByAccountId(u64 account_id, s32* user_id);
s32 sceNpGetUserIdByOnlineId(const OrbisNpOnlineId* online_id, s32* user_id);
s32 sceNpHasSignedUp(s32 user_id, bool* has_signed_up);
s32 sceNpIsPlusMember(s32 user_id, bool* result);
s32 sceNpSetContentRestriction(const OrbisNpContentRestriction* restriction);
s32 sceNpSetGamePresenceOnline(s32 req_id, const OrbisNpOnlineId* online_id, u32 presence);
s32 sceNpSetGamePresenceOnlineA(s32 req_id, s32 user_id, u32 presence);
s32 sceNpSetTimeout(s32 req_id, s32 resolve_retry, u32 resolve_timeout, u32 conn_timeout,
                    u32 send_timeout, u32 recv_timeout);
void sceNpRegisterGamePresenceCallback(OrbisNpGamePresenceCallback callback, void* userdata);
s32 sceNpRegisterGamePresenceCallbackA(OrbisNpGamePresenceCallback callback, void* userdata);
s32 sceNpUnregisterGamePresenceCallbackA(s32 callback_id);
s32 sceNpRegisterPlusEventCallback(OrbisNpPlusEventCallback callback, void* userdata);
s32 sceNpUnregisterPlusEventCallback();

void RegisterNpCallback(std::string key, std::function<void()> cb);
void QueueNpStateEvent(s32 user_id, OrbisNpState state);
void UpdateNpStateFromEvent(s32 event_type, s32 user_id);
void RegisterHooks();
} // namespace Libraries::Np::NpManager