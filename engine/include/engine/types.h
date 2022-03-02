/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifndef VOLKANO_TYPES_H
#define VOLKANO_TYPES_H

#include <cstdint>
#include <cstddef>

namespace volkano {

using i8 = std::int8_t;
using u8 = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using usize = std::size_t;
using ssize = std::ptrdiff_t;

using f32 = float;
using f64 = double;

} // namespace volkano

#endif // VOLKANO_TYPES_H
