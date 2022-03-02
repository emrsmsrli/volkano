/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifndef VOLKANO_CONSTANTS_H
#define VOLKANO_CONSTANTS_H

#include <concepts>
#include <numbers>

#include "engine/types.h"

namespace volkano::math::consts {

template<std::floating_point Floating> inline constexpr Floating half_pi_v = std::numbers::pi_v<Floating> / Floating(2.0);
template<std::floating_point Floating> inline constexpr Floating quarter_pi_v = std::numbers::pi_v<Floating> / Floating(4.0);
template<std::floating_point Floating> inline constexpr Floating two_over_pi_v = Floating(2.0) / std::numbers::pi_v<Floating>;
template<std::floating_point Floating> inline constexpr Floating two_over_sqrt_pi_v = 1.12837916709551257390;
template<std::floating_point Floating> inline constexpr Floating inv_sqrt2_v = Floating(1.0) / std::numbers::sqrt2_v<Floating>;

inline constexpr f32 small_float      = 0.00001f;
inline constexpr f32 e                = std::numbers::e_v<f32>;
inline constexpr f32 log2e            = std::numbers::log2e_v<f32>;
inline constexpr f32 log10e           = std::numbers::log10e_v<f32>;
inline constexpr f32 pi               = std::numbers::pi_v<f32>;
inline constexpr f32 inv_pi           = std::numbers::inv_pi_v<f32>;
inline constexpr f32 inv_sqrtpi       = std::numbers::inv_sqrtpi_v<f32>;
inline constexpr f32 ln2              = std::numbers::ln2_v<f32>;
inline constexpr f32 ln10             = std::numbers::ln10_v<f32>;
inline constexpr f32 sqrt2            = std::numbers::sqrt2_v<f32>;
inline constexpr f32 sqrt3            = std::numbers::sqrt3_v<f32>;
inline constexpr f32 inv_sqrt3        = std::numbers::inv_sqrt3_v<f32>;
inline constexpr f32 half_pi          = half_pi_v<f32>;
inline constexpr f32 quarter_pi       = quarter_pi_v<f32>;
inline constexpr f32 two_over_pi      = two_over_pi_v<f32>;
inline constexpr f32 two_over_sqrt_pi = two_over_sqrt_pi_v<f32>;
inline constexpr f32 inv_sqrt2        = two_over_sqrt_pi_v<f32>;

} // namespace volkano::math::consts

#endif // VOLKANO_CONSTANTS_H
