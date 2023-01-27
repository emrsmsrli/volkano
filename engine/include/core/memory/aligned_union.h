/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <algorithm>
#include <concepts>
#include <new>
#include <utility>

#include "core/int_types.h"
#include "core/type_traits.h"

namespace volkano {

template<typename... Ts>
class aligned_union {
public:
    static_assert(sizeof...(Ts) > 0, "must provide at least one type");
    static_assert(!is_one_of_v<void, Ts...>, "type 'void' cannot be in the union type list");

    static inline constexpr usize max_size = std::max({sizeof(Ts)...});
    static inline constexpr usize max_alignment = std::max({alignof(Ts)...});

private:
    alignas(max_alignment) std::array<std::byte, max_size> storage_;

public:
    template<typename T, typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr T* construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        check_type<T>();
        std::construct_at(reinterpret_cast<T*>(storage_.data()), std::forward<Args>(args)...);
        return value<T>();
    }

    template<typename T>
    constexpr void destruct() noexcept(std::is_nothrow_destructible_v<T>)
    {
        check_type<T>();
        if constexpr (!std::is_trivially_destructible_v<T>) {
            std::destroy_at(value<T>());
        }
    }

    template<typename T = type_list_element_t<0, type_list<Ts...>>>
    constexpr T* value() noexcept
    {
        check_type<T>();
        return std::launder(reinterpret_cast<T*>(storage_.data()));
    }

    template<typename T = type_list_element_t<0, type_list<Ts...>>>
    constexpr const T* value() const noexcept
    {
        check_type<T>();
        return std::launder(reinterpret_cast<const T*>(storage_.data()));
    }

private:
    template<typename T>
    static constexpr void check_type() noexcept
    {
        static_assert(is_one_of_v<T, Ts...>, "T is a not a part of this union");
    }
};

} // namespace volkano
