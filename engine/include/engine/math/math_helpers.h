/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifndef VOLKANO_MATH_HELPERS_H
#define VOLKANO_MATH_HELPERS_H

#include <concepts>
#include <cmath>

#include "engine/types.h"
#include "engine/math/constants.h"

namespace volkano::math {

template<typename T>
constexpr T square(const T val) noexcept
{
    return val * val;
}

template<std::floating_point Fp>
bool is_nearly_equal(const Fp l, const Fp r, const Fp epsilon = consts::small_float) noexcept
{
    return std::abs(l - r) < epsilon;
}

template<std::floating_point Fp>
constexpr Fp to_radians(const Fp degrees) noexcept
{
    return degrees * consts::pi<Fp> / T(180.0);
}

template<std::floating_point Fp>
constexpr Fp to_degrees(const Fp radians) noexcept
{
    return radians * Fp(180.0) / consts::pi_v<Fp>;
}

} // namespace volkano::math

#endif // VOLKANO_MATH_HELPERS_H
