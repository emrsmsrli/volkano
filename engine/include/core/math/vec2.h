/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "core/assert.h"
#include "math_helpers.h"

namespace volkano {

template<typename T>
struct vec2 {
    T x;
    T y;

    [[nodiscard]] constexpr T length_sq() const noexcept { return math::square(x) + math::square(y); }
    [[nodiscard]] constexpr T length() const noexcept { return std::sqrt(length_sq()); }

    [[nodiscard]] constexpr T dot(const vec2& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    constexpr void normalize() noexcept { *this /= length(); }

    constexpr bool normalize_safe() noexcept
    {
        if (is_nearly_zero()) {
            return false;
        }

        normalize();
        return true;
    }

    [[nodiscard]] constexpr vec2 get_normalized() const noexcept
    {
        vec2 copy = *this;
        copy.normalize();
        return copy;
    }

    [[nodiscard]] constexpr vec2 get_normalized_safe() const noexcept
    {
        if (is_nearly_zero()) {
            return zero;
        }
        return get_normalized();
    }

    constexpr T operator[](u32 idx) const noexcept
    {
        switch (idx) {
            case 0: return x;
            case 1: return y;
            default: VKE_UNREACHABLE();
        }
    }

    constexpr T& operator[](u32 idx) noexcept
    {
        switch (idx) {
            case 0: return x;
            case 1: return y;
            default: VKE_UNREACHABLE();
        }
    }

    constexpr vec2 operator+(const vec2& other) const noexcept { return {.x = x + other.x, .y = y + other.y}; }
    constexpr vec2 operator-(const vec2& other) const noexcept { return {.x = x - other.x, .y = y - other.y}; }
    constexpr vec2 operator*(const T scalar) const noexcept { return {.x = x * scalar, .y = y * scalar}; }

    constexpr vec2 operator/(const T scalar) const noexcept
    {
        VKE_ASSERT(scalar != T(0));
        if constexpr (std::floating_point<T>) {
            const T inverse_scalar = T(1) / scalar;
            return *this * inverse_scalar;
        } else {
            return {.x = x / scalar, .y = y / scalar};
        }
    }

    constexpr vec2& operator+=(const vec2& other) noexcept { *this = *this + other; return *this; }
    constexpr vec2& operator-=(const vec2& other) noexcept { *this = *this - other; return *this; }
    constexpr vec2& operator*=(const T scalar) noexcept { *this = *this * scalar; return *this; }
    constexpr vec2& operator/=(const T scalar) noexcept { *this = *this / scalar; return *this; }

    constexpr bool operator==(const vec2& other) const noexcept { return x == other.x && y == other.y; };
    constexpr bool operator!=(const vec2& other) const noexcept { return !(*this == other); };
    [[nodiscard]] constexpr bool is_unit() const noexcept { return length_sq() == T(1); };
    [[nodiscard]] constexpr bool is_zero() const noexcept { return *this == zero(); };
    [[nodiscard]] constexpr bool has_nan() const noexcept requires std::floating_point<T> { return math::any_nans(x, y); }
    [[nodiscard]] constexpr bool is_nearly_zero() const noexcept { return math::is_nearly_zero(x) && math::is_nearly_zero(y); };

    static constexpr vec2 unit_x() noexcept { return {.x = T(1), .y = T(0)}; }
    static constexpr vec2 unit_y() noexcept { return {.x = T(0), .y = T(1)}; }
    static constexpr vec2 zero() noexcept { return {.x = T(0), .y = T(0)}; }
    static constexpr vec2 one() noexcept { return {.x = T(1), .y = T(1)}; }

    static constexpr vec2 from_same(const T component) noexcept { return {.x = component, .y = component}; }
    static constexpr vec2 from_radians(const T radians) noexcept { return {.x = radians, .y = radians}; }    // todo
    static constexpr vec2 from_degrees(const T degrees) noexcept { return {.x = degrees, .y = degrees}; }    // todo
};

template<typename T>
constexpr T angle_between(const vec2<T>& l, const vec2<T>& r) noexcept
{
    VKE_ASSERT(l.is_unit() && r.is_unit());
    return std::atan(l.dot(r));
}

using vec2f = vec2<f32>;
using vec2f64 = vec2<f64>;
using vec2i = vec2<i32>;
using vec2u = vec2<u32>;

} // namespace volkano
