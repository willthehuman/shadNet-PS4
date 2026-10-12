// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <orbis/NpManager.h>
#include "common/types.h"

namespace Libraries::Np::NpManager {

constexpr s32 ORBIS_NP_MANAGER_REQUEST_LIMIT = 0x20;
constexpr s32 ORBIS_NP_MANAGER_REQUEST_ID_OFFSET = 0x20000000;

struct OrbisNpCreateAsyncRequestParameter {
    u64 size;
    u64 cpu_affinity_mask;
    s32 thread_priority;
    u8 padding[4];
};

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

void RegisterNpCallback(std::string key, std::function<void()> cb);
void QueueNpStateEvent(s32 user_id, OrbisNpState state);
void UpdateNpStateFromEvent(s32 event_type, s32 user_id);
void RegisterHooks();
} // namespace Libraries::Np::NpManager