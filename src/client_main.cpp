// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <orbis/Sysmodule.h>
#include <orbis/UserService.h>
#include <orbis/libkernel.h>
#include "client_main.h"
#include "common/elf_info.h"
#include "common/logging/log.h"
#include "common/types.h"
#include "core/libraries/kernel/kernel.h"
#include "core/libraries/network/net.h"
#include "core/libraries/network/http_hooks.h"
#include "core/libraries/network/netctl_hooks.h"
#include "core/libraries/np/np_handler.h"
#include "core/libraries/np/np_auth.h"
#include "core/libraries/np/np_manager/np_manager.h"
#include "core/libraries/np/np_matching2/np_matching2.h"
#include "core/libraries/np/np_score.h"
#include "core/libraries/np/np_signaling/np_signaling.h"
#include "core/libraries/np/np_web_api/np_web_api.h"
#include "core/libraries/system/user_service.h"
#include "shadnet/config.h"

// The GoldHEN plugin loader never invokes .init_array for plugins (the SDK's
// crt/crtprx.c `_init` calls module_start directly), so run the module's
// static initializers ourselves before anything else: without this pass every
// C++ static constructor in the module is skipped and globals (like the
// matching2 client state) stay zero-initialized, faulting on first use.
extern "C" {
extern void (*__init_array_start[])(void) __attribute__((visibility("hidden")));
extern void (*__init_array_end[])(void) __attribute__((visibility("hidden")));
}

static void RunPluginStaticInitializers() {
    const auto count = static_cast<size_t>(__init_array_end - __init_array_start);
    size_t ran = 0;
    for (size_t i = 0; i < count; ++i) {
        void (*fn)() = __init_array_start[i];
        const auto value = reinterpret_cast<uintptr_t>(fn);
        if (!fn) {
            continue;
        }
        // Only call entries relocated to the module's runtime address range.
        // Raw link-time values (if relocations were ever skipped) are rejected
        // instead of jumping into unmapped memory.
        if (value >= 0x10000000) {
            LOG_INFO(shadNet, "  static init [{}/{}] @ 0x{:x}", i + 1, count, value);
            fn();
            ++ran;
        }
    }
    LOG_INFO(shadNet, "static initializers: ran {}/{}", ran, count);
}

extern "C" s32 client_start() {
    RunPluginStaticInitializers();
    LOG_INFO(shadNet, "Initializing shadNet Client");
    // Preload modules used by the plugin
    sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_NET);
    sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_NETCTL);
    sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_SYS_UTIL);
    sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_USER_SERVICE);
    sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_NP_MANAGER);
    sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_NP_WEB_API);

    sceSysmoduleLoadModule(ORBIS_SYSMODULE_NP_SCORE_RANKING);
    sceSysmoduleLoadModule(ORBIS_SYSMODULE_NP_MATCHING2);
    sceSysmoduleLoadModule(ORBIS_SYSMODULE_NP_SIGNALING);

    // Init kernel and UserService hooks
    Libraries::Kernel::Kernel::RegisterHooks();
    Libraries::System::UserService::RegisterHooks();

    // Init config backend
    ShadNet::Settings::GetInstance().Initialize();

    // Init other library hooks
    Libraries::Network::Net::RegisterHooks();
    Libraries::Network::Http::RegisterHooks();
    Libraries::Network::NetCtl::RegisterHooks();
    Libraries::Np::NpManager::RegisterHooks();
    Libraries::Np::NpAuth::RegisterHooks();
    Libraries::Np::NpScore::RegisterHooks();
    Libraries::Np::NpWebApi::RegisterHooks();
    Libraries::Np::NpMatching2::RegisterHooks();
    Libraries::Np::NpSignaling::RegisterHooks();

    // Initialize elfinfo
    auto& game_info = Common::ElfInfo::Instance();

    // Retrieve as much game metadata as we can
    OrbisAppInfo app_info{};
    s32 result = sceKernelGetAppInfo(getpid(), &app_info);
    if (result != 0) {
        LOG_ERROR(shadNet, "sceKernelGetAppInfo failed!");
        return 1;
    }

    LOG_INFO(shadNet, "Currently running {}", app_info.TitleId);
    sceKernelGetCompiledSdkVersion(reinterpret_cast<s32*>(&game_info.sdk_ver));
    game_info.initialized = true;
    game_info.game_serial = std::string{app_info.TitleId};

    // Initialize NpHandler
    Libraries::Np::NpHandler::Instance().Initialize();
    return 0;
}