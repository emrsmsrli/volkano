/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <concepts>
#include <cmath>

#include "core/int_types.h"
#include "constants.h"

namespace volkano::math {

template<typename T>
constexpr T square(const T val) noexcept
{
    return val * val;
}

template<std::floating_point... T>
constexpr bool any_nans(const T... vals) noexcept
{
    return (std::isnan(vals) || ...);
}

template<std::floating_point Fp>
bool is_nearly_equal(const Fp l, const Fp r, const Fp epsilon = consts::small_float) noexcept
{
    return std::abs(l - r) < epsilon;
}

template<std::floating_point Fp>
bool is_nearly_zero(const Fp l, const Fp epsilon = consts::small_float) noexcept
{
    return std::abs(l) < epsilon;
}

template<std::floating_point Fp>
constexpr Fp to_radians(const Fp degrees) noexcept
{
    return degrees * std::numbers::pi_v<Fp> / Fp(180.0);
}

template<std::floating_point Fp>
constexpr Fp to_degrees(const Fp radians) noexcept
{
    return radians * Fp(180.0) / std::numbers::pi_v<Fp>;
}

} // namespace volkano::math
