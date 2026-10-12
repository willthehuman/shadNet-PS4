// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Plugin-side rewrite of the shadps4 np_common helpers used by the ported NP
// modules (currently the signalling layer). The shadps4 originals bind to the
// emulator's kernel pthread shims; here the same public surface is implemented
// with plain STL primitives, which behave identically at game runtime:
//  - mutex handles are heap std::recursive_mutex objects (a superset of the
//    non-recursive lw-mutex semantics, and immune to the plugin's load-time
//    pthread restrictions since everything here is created at runtime),
//  - condition handles are std::condition_variable_any (works with the
//    recursive mutexes the callout context uses),
//  - "threads" are std::thread objects behind opaque handle pointers.

#include "core/libraries/np/np_common.h"

#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <new>
#include <thread>

#include "common/logging/log.h"
#include "core/libraries/np/np_error.h"

namespace Libraries::Np::NpCommon {

namespace {

struct MutexHandle {
    std::recursive_mutex m;
};

struct CondHandle {
    std::condition_variable_any cv;
};

MutexHandle* AsMutex(Libraries::Kernel::PthreadMutexT handle) {
    return static_cast<MutexHandle*>(handle);
}

CondHandle* AsCond(Libraries::Kernel::PthreadCondT handle) {
    return static_cast<CondHandle*>(handle);
}

s64 NowUsec() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

void* PS4_SYSV_ABI CalloutThreadMain(void* arg);

} // namespace

u32 PS4_SYSV_ABI sceNpMutexLock(Libraries::Kernel::PthreadMutexT* mutex) {
    AsMutex(*mutex)->m.lock();
    return ORBIS_OK;
}

u32 PS4_SYSV_ABI sceNpMutexUnlock(Libraries::Kernel::PthreadMutexT* mutex) {
    AsMutex(*mutex)->m.unlock();
    return ORBIS_OK;
}

u32 PS4_SYSV_ABI sceNpMutexInit(Libraries::Kernel::PthreadMutexT* mutex, const char* /*name*/,
                                u64 /*flags*/) {
    if (mutex == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    *mutex = new (std::nothrow) MutexHandle{};
    return *mutex != nullptr ? ORBIS_OK : ORBIS_NP_ERROR_OUT_OF_MEMORY;
}

s32 PS4_SYSV_ABI sceNpMutexDestroy(Libraries::Kernel::PthreadMutexT* mutex) {
    if (mutex == nullptr || *mutex == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    delete AsMutex(*mutex);
    *mutex = nullptr;
    return ORBIS_OK;
}

u32 PS4_SYSV_ABI sceNpMutexTryLock(Libraries::Kernel::PthreadMutexT* mutex) {
    if (AsMutex(*mutex)->m.try_lock()) {
        return ORBIS_OK;
    }
    return ORBIS_NP_LW_MUTEX_ERROR_BUSY;
}

u32 PS4_SYSV_ABI sceNpLwMutexLock(Libraries::Kernel::PthreadMutexT* mutex) {
    return sceNpMutexLock(mutex);
}

u32 PS4_SYSV_ABI sceNpLwMutexUnlock(Libraries::Kernel::PthreadMutexT* mutex) {
    return sceNpMutexUnlock(mutex);
}

u32 PS4_SYSV_ABI sceNpLwMutexInit(Libraries::Kernel::PthreadMutexT* mutex, const char* name,
                                  u64 flags) {
    return sceNpMutexInit(mutex, name, flags);
}

s32 PS4_SYSV_ABI sceNpLwMutexDestroy(Libraries::Kernel::PthreadMutexT* mutex) {
    return sceNpMutexDestroy(mutex);
}

u32 PS4_SYSV_ABI sceNpLwMutexTryLock(Libraries::Kernel::PthreadMutexT* mutex) {
    return sceNpMutexTryLock(mutex);
}

s32 PS4_SYSV_ABI sceNpCondInit(Libraries::Kernel::PthreadCondT* cond, const char* /*name*/,
                               u64 /*flags*/) {
    if (cond == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    *cond = new (std::nothrow) CondHandle{};
    return *cond != nullptr ? ORBIS_OK : ORBIS_NP_ERROR_OUT_OF_MEMORY;
}

s32 PS4_SYSV_ABI sceNpCondDestroy(Libraries::Kernel::PthreadCondT* cond) {
    if (cond == nullptr || *cond == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    delete AsCond(*cond);
    *cond = nullptr;
    return ORBIS_OK;
}

s32 PS4_SYSV_ABI sceNpCondSignal(Libraries::Kernel::PthreadCondT* cond) {
    AsCond(*cond)->cv.notify_all();
    return ORBIS_OK;
}

s32 PS4_SYSV_ABI sceNpCondTimedwait(Libraries::Kernel::PthreadCondT* cond,
                                    Libraries::Kernel::PthreadMutexT* mutex, u32 usec) {
    auto* cond_handle = AsCond(*cond);
    auto* mutex_handle = AsMutex(*mutex);
    // Called with the mutex held (pthread_cond_timedwait contract): adopt the
    // existing lock, wait, and hand the lock back still held.
    std::unique_lock<std::recursive_mutex> lock(mutex_handle->m, std::adopt_lock);
    if (usec == 0) {
        cond_handle->cv.wait(lock);
    } else {
        cond_handle->cv.wait_for(lock, std::chrono::microseconds(usec));
    }
    lock.release();
    return ORBIS_OK;
}

s32 PS4_SYSV_ABI sceNpCreateThread(Libraries::Kernel::PthreadT* thread, NpThreadEntry start_routine,
                                   void* arg, s32 priority, u64 stack_size, u64 affinity_mask,
                                   const char* name) {
    if (thread == nullptr || start_routine == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    LOG_DEBUG(Lib_NpCommon,
              "CreateThread name={} priority={} stack={:#x} affinity={:#x} (scheduling hints "
              "ignored)",
              name ? name : "?", priority, stack_size, affinity_mask);
    auto* t = new (std::nothrow) std::thread([start_routine, arg]() { start_routine(arg); });
    if (t == nullptr) {
        return ORBIS_NP_ERROR_OUT_OF_MEMORY;
    }
    *thread = t;
    return ORBIS_OK;
}

u32 PS4_SYSV_ABI sceNpJoinThread(Libraries::Kernel::PthreadT thread, void** ret) {
    if (thread == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    auto* t = static_cast<std::thread*>(thread);
    if (t->joinable()) {
        t->join();
    }
    delete t;
    if (ret != nullptr) {
        *ret = nullptr;
    }
    return ORBIS_OK;
}

s32 PS4_SYSV_ABI sceNpGetSystemClockUsec(s64* usec) {
    if (usec == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
    *usec = NowUsec();
    return ORBIS_OK;
}

s32 PS4_SYSV_ABI sceNpGetPlatformType(const OrbisNpId* npid) {
    if (npid == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }

    static constexpr u32 kTagPs3 = 0x00337370;  // "ps3"
    static constexpr u32 kTagVita = 0x32707370; // "psp2(VITA)"
    static constexpr u32 kTagPs4 = 0x00347370;  // "ps4"

    if (npid->opt[4] == 0) {
        return 0;
    }
    u32 tag = 0;
    std::memcpy(&tag, npid->opt + 4, sizeof(tag));
    switch (tag) {
    case kTagPs3:
        return 1;
    case kTagVita:
        return 2;
    case kTagPs4:
        return 3;
    default:
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }
}

s32 PS4_SYSV_ABI sceNpIntIsValidOnlineId(const OrbisNpOnlineId* id) {
    if (id == nullptr || id->term != 0) {
        return 0;
    }

    const size_t len = strnlen(id->data, ORBIS_NP_ONLINEID_MAX_LENGTH + 1);
    if (len < 3 || len > ORBIS_NP_ONLINEID_MAX_LENGTH) {
        return 0;
    }

    const unsigned char first = static_cast<unsigned char>(id->data[0]);
    if (!std::isalnum(first)) {
        return 0;
    }

    for (size_t i = 1; i < len; ++i) {
        const unsigned char c = static_cast<unsigned char>(id->data[i]);
        if (std::isalnum(c) || c == '_' || c == '-') {
            continue;
        }
        return 0;
    }

    return 1;
}

void PS4_SYSV_ABI sceNpGetSdkVersion(char* version_buf) {
    if (version_buf == nullptr) {
        return;
    }
    std::snprintf(version_buf, 8, "09.03");
}

namespace {

void* PS4_SYSV_ABI CalloutThreadMain(void* arg) {
    auto* ctx = static_cast<OrbisNpCalloutContext*>(arg);
    auto* mutex = AsMutex(ctx->mutex);
    mutex->m.lock();

    for (;;) {
        const s64 now_usec = NowUsec();

        auto* entry = ctx->head;
        u32 wait_usec = 0;

        if (entry == nullptr) {
            if (ctx->stop_requested > 0) {
                break;
            }
        } else {
            const s64 remaining = entry->deadline_usec - now_usec;
            if (remaining <= 0) {
                ctx->head = entry->next;
                mutex->m.unlock();
                if (entry->handler != nullptr) {
                    entry->handler(entry->arg);
                }
                mutex->m.lock();
                continue;
            }

            if (ctx->stop_requested > 0) {
                break;
            }

            wait_usec = static_cast<u32>(remaining > 0xffffffffLL ? 0xffffffffLL : remaining);
        }

        // Wait on the condition with the mutex held (releases during the wait,
        // reacquires afterwards - the standard condition-wait contract).
        auto* cond_handle = AsCond(ctx->cond);
        std::unique_lock<std::recursive_mutex> lock(mutex->m, std::adopt_lock);
        if (wait_usec == 0) {
            cond_handle->cv.wait(lock);
        } else {
            cond_handle->cv.wait_for(lock, std::chrono::microseconds(wait_usec));
        }
        lock.release();

        if (ctx->stop_requested > 0) {
            break;
        }
    }

    mutex->m.unlock();
    return nullptr;
}

} // namespace

s32 PS4_SYSV_ABI sceNpCalloutInitCtx(OrbisNpCalloutContext* callout_ctx, const char* name,
                                     u64 stack_size, s32 priority, u64 affinity_mask) {
    if (callout_ctx == nullptr) {
        return ORBIS_NP_ERROR_INVALID_ARGUMENT;
    }

    s32 rc = ORBIS_NP_CALLOUT_ERROR_ALREADY_INITIALIZED;
    if (callout_ctx->active < 1) {
        callout_ctx->stop_requested = 0;
        callout_ctx->head = nullptr;
        rc = static_cast<s32>(sceNpMutexInit(&callout_ctx->mutex, name, 1));
        if (rc >= 0) {
            rc = sceNpCondInit(&callout_ctx->cond, name, 0);
            if (rc >= 0) {
                rc = sceNpCreateThread(&callout_ctx->thread, CalloutThreadMain, callout_ctx,
                                       priority, stack_size, affinity_mask, name);
                if (rc >= 0) {
                    callout_ctx->active = 1;
                    return ORBIS_OK;
                }
                sceNpCondDestroy(&callout_ctx->cond);
            }
            sceNpMutexDestroy(&callout_ctx->mutex);
        }
    }
    return rc;
}

namespace {

s32 CalloutStartOnCtx64Internal(OrbisNpCalloutContext* callout_ctx, OrbisNpCalloutEntry* entry,
                                s64 delay_usec, u64 handler, u64 arg) {
    if (callout_ctx->active == 0) {
        return ORBIS_NP_CALLOUT_ERROR_NOT_INITIALIZED;
    }

    entry->next = nullptr;
    entry->handler = reinterpret_cast<OrbisNpCalloutHandler>(handler);
    entry->arg = arg;
    entry->deadline_usec = NowUsec() + delay_usec;

    sceNpMutexLock(&callout_ctx->mutex);

    for (auto* cur = callout_ctx->head; cur != nullptr; cur = cur->next) {
        if (cur == entry) {
            sceNpMutexUnlock(&callout_ctx->mutex);
            return ORBIS_NP_CALLOUT_ERROR_DUPLICATE_ENTRY;
        }
    }

    OrbisNpCalloutEntry** prev_next = &callout_ctx->head;
    for (auto* cur = callout_ctx->head; cur != nullptr; cur = cur->next) {
        if (cur->deadline_usec >= entry->deadline_usec) {
            break;
        }
        prev_next = &cur->next;
    }

    entry->next = *prev_next;
    *prev_next = entry;
    sceNpCondSignal(&callout_ctx->cond);
    sceNpMutexUnlock(&callout_ctx->mutex);
    return ORBIS_OK;
}

} // namespace

s32 PS4_SYSV_ABI sceNpCalloutStartOnCtx(OrbisNpCalloutContext* callout_ctx,
                                        OrbisNpCalloutEntry* entry, u64 delay_usec, u64 handler,
                                        u64 arg) {
    return CalloutStartOnCtx64Internal(callout_ctx, entry,
                                       static_cast<s64>(static_cast<u32>(delay_usec)), handler,
                                       arg);
}

s32 PS4_SYSV_ABI sceNpCalloutStartOnCtx64(OrbisNpCalloutContext* callout_ctx,
                                          OrbisNpCalloutEntry* entry, s64 delay_usec, u64 handler,
                                          u64 arg) {
    return CalloutStartOnCtx64Internal(callout_ctx, entry, delay_usec, handler, arg);
}

s32 PS4_SYSV_ABI sceNpCalloutStopOnCtx(OrbisNpCalloutContext* callout_ctx,
                                       OrbisNpCalloutEntry* entry, u32* removed) {
    if (callout_ctx->active == 0) {
        return ORBIS_NP_CALLOUT_ERROR_NOT_INITIALIZED;
    }

    sceNpMutexLock(&callout_ctx->mutex);
    u32 found = 0;
    OrbisNpCalloutEntry** prev_next = &callout_ctx->head;
    for (auto* cur = callout_ctx->head; cur != nullptr; cur = cur->next) {
        if (cur == entry) {
            *prev_next = cur->next;
            found = 1;
            break;
        }
        prev_next = &cur->next;
    }
    if (removed != nullptr) {
        *removed = found;
    }
    sceNpMutexUnlock(&callout_ctx->mutex);
    return ORBIS_OK;
}

void PS4_SYSV_ABI sceNpCalloutTermCtx(OrbisNpCalloutContext* callout_ctx) {
    if (callout_ctx == nullptr || callout_ctx->active == 0) {
        return;
    }
    sceNpMutexLock(&callout_ctx->mutex);
    callout_ctx->stop_requested = 1;
    sceNpCondSignal(&callout_ctx->cond);
    sceNpMutexUnlock(&callout_ctx->mutex);
    sceNpJoinThread(callout_ctx->thread, nullptr);
    sceNpCondDestroy(&callout_ctx->cond);
    sceNpMutexDestroy(&callout_ctx->mutex);
    callout_ctx->active = 0;
}

} // namespace Libraries::Np::NpCommon
