/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifndef VOLKANO_ALIGNED_UNION_H
#define VOLKANO_ALIGNED_UNION_H

#include <array>
#include <utility>

#include "engine/types.h"
#include "engine/core/template.h"

namespace volkano {

template<typename... Ts>
struct aligned_union {
    static constexpr usize max_size = std::max({sizeof(Ts)...});

    template<typename T, typename... Args>
    T* construct(Args&&... args)
    {
        check_type<T>();
        new (storage.data()) T(std::forward<Args>(args)...);
        return data_as<T>();
    }

    template<typename T>
    void destruct()
    {
        if constexpr(!std::is_trivially_destructible_v<T>) {
            check_type<T>();
            T* data = data_as<T>();
            data->~T();
        }
    }

    template<typename T>
    T* data_as() noexcept
    {
        check_type<T>();
        return reinterpret_cast<T*>(storage.data());
    }

    template<typename T>
    const T* data_as() const noexcept
    {
        check_type<T>();
        return reinterpret_cast<T*>(storage.data());
    }

private:
    alignas(Ts...) std::array<std::byte, max_size> storage;

    template<typename T>
    static void check_type() noexcept
    {
        static_assert(is_one_of_v<T, Ts...>, "T is a not a part of this union");
    }
};

} // namespace volkano

#endif // VOLKANO_ALIGNED_UNION_H
