// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Minimal type shims for the shadps4 kernel pthread handles used by the ported
// NP modules. Handles are opaque pointers to plugin-side objects created by
// np_common (see np_common.cpp).
namespace Libraries::Kernel {
using PthreadT = void*;
using PthreadMutexT = void*;
using PthreadCondT = void*;
using PthreadAttrT = void*;
using PthreadMutexAttrT = void*;
using PthreadCondAttrT = void*;
} // namespace Libraries::Kernel
