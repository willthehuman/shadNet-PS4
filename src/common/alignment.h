// SPDX-FileCopyrightText: Copyright 2026 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <type_traits>

namespace Common {

template <typename T, typename A>
constexpr T AlignUp(T value, A alignment) {
    return (value + static_cast<T>(alignment) - 1) & ~(static_cast<T>(alignment) - 1);
}

} // namespace Common
