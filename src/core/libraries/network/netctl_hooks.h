// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

namespace Libraries::Network::NetCtl {

// Installs the game-facing netctl hooks (sceNetCtl* family).
void RegisterHooks();

} // namespace Libraries::Network::NetCtl
