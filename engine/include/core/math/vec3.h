/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "core/assert.h"
#include "core/math/math_helpers.h"

namespace volkano {

template<typename T>
struct vec3 {
    T x;
    T y;
    T z;

    [[nodiscard]] constexpr T length_sq() const noexcept { return math::square(x) + math::square(y) + math::square(z); }
    [[nodiscard]] constexpr T length() const noexcept { return std::sqrt(length_sq()); }

    [[nodiscard]] constexpr T dot(const vec3& other) const noexcept
    {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr vec3 cross(const vec3& other) const noexcept
    {
        return {
          .x = y * other.z - z * other.y,
          .y = z * other.x - x * other.z,
          .z = x * other.y - y * other.x
        };
    }

    constexpr void normalize() noexcept { *this /= length(); }

    constexpr bool normalize_safe() noexcept
    {
        if (is_zero()) { // todo check nearly zero
            return false;
        }

        normalize();
        return true;
    }

    [[nodiscard]] constexpr vec3 get_normalized() const noexcept
    {
        vec3 copy = *this;
        copy.normalize();
        return copy;
    }

    [[nodiscard]] constexpr vec3 get_normalized_safe() const noexcept
    {
        if (is_zero()) { // todo check nearly zero
            return zero;
        }

        return get_normalized();
    }

    constexpr T operator[](u32 idx) const noexcept
    {
        switch (idx) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: VKE_UNREACHABLE();
        }
    }

    constexpr T& operator[](u32 idx) noexcept
    {
        switch (idx) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: VKE_UNREACHABLE();
        }
    }

    constexpr vec3 operator+(const vec3& other) const noexcept { return {.x = x + other.x, .y = y + other.y, .z = z + other.z}; }
    constexpr vec3 operator-(const vec3& other) const noexcept { return {.x = x - other.x, .y = y - other.y, .z = z - other.z}; }
    constexpr vec3 operator*(const T scalar) const noexcept { return {.x = x * scalar, .y = y * scalar, .z = z * scalar}; }

    constexpr vec3 operator/(const T scalar) const noexcept
    {
        VKE_ASSERT(scalar != T(0));
        if constexpr (std::floating_point<T>) {
            const T inverse_scalar = T(1) / scalar;
            return *this * inverse_scalar;
        } else {
            return {.x = x / scalar, .y = y / scalar, .z = z / scalar};
        }
    }

    constexpr vec3& operator+=(const vec3& other) noexcept { *this = *this + other; return *this; }
    constexpr vec3& operator-=(const vec3& other) noexcept { *this = *this - other; return *this; }
    constexpr vec3& operator*=(const T scalar) noexcept { *this = *this * scalar; return *this; }
    constexpr vec3& operator/=(const T scalar) noexcept { *this = *this / scalar; return *this; }

    constexpr bool operator==(const vec3& other) const noexcept { return x == other.x && y == other.y && z == other.z; };
    constexpr bool operator!=(const vec3& other) const noexcept { return !(*this == other); };
    [[nodiscard]] constexpr bool is_unit() const noexcept { return length_sq() == T(1); };
    [[nodiscard]] constexpr bool is_zero() const noexcept { return *this == zero(); };
    [[nodiscard]] constexpr bool has_nan() const noexcept requires std::floating_point<T> { return std::isnan(x) || std::isnan(y); }

    static constexpr vec3 unit_x() noexcept { return {.x = T(1), .y = T(0), .z = T(0)}; }
    static constexpr vec3 unit_y() noexcept { return {.x = T(0), .y = T(1), .z = T(0)}; }
    static constexpr vec3 unit_z() noexcept { return {.x = T(0), .y = T(0), .z = T(1)}; }
    static constexpr vec3 zero() noexcept { return {.x = T(0), .y = T(0), .z = T(0)}; }
    static constexpr vec3 one() noexcept { return {.x = T(1), .y = T(1), .y = T(1)}; }

    static constexpr vec3 from_same(const T component) noexcept { return {.x = component, .y = component, .z = component}; }
    static constexpr vec3 from_radians(const T radians) noexcept { return {.x = radians, .y = radians, .z = radians}; }    // todo
    static constexpr vec3 from_degrees(const T degrees) noexcept { return {.x = degrees, .y = degrees, .z = degrees}; }    // todo
};

/** assumes parameters are unit vectors */
template<typename T>
constexpr T angle_between(const vec3<T>& l, const vec3<T>& r) noexcept
{
    VKE_ASSERT(l.is_unit() && r.is_unit());
    return std::atan(l.dot(r));
}

using vec3i = vec3<i32>;
using vec3u = vec3<u32>;
using vec3f = vec3<f32>;

} // namespace volkano
