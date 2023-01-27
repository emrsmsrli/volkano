/*
 * Copyright (C) 2023 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <string_view>

#include "core/int_types.h"

namespace volkano {

[[maybe_unused]] constexpr u8 version_major = 0;
[[maybe_unused]] constexpr u8 version_minor = 0;
[[maybe_unused]] constexpr u8 version_patch = 1;

[[maybe_unused]] constexpr u8 version_int = version_major << 16 | version_minor << 8 | version_patch;

[[maybe_unused]] constexpr std::string_view version = "0.0.1";

} // namespace volkano
