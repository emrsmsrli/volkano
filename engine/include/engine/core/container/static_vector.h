/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <type_traits>
#include <new>

#include "engine/core/int_types.h"
#include "engine/core/assert.h"
#include "engine/core/memory/aligned_union.h"

namespace volkano {

template<typename T, usize Capacity>
    requires (Capacity != 0 && !std::is_void_v<T>)
class static_vector {
    static_assert(Capacity != 0, "Vector capacity must not be zero");

    using storage_t = std::array<aligned_union<T>, Capacity>;

    storage_t storage_;
    usize size_ = 0;

public:
    using value_type = T;
    using size_type = usize;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = typename storage_t::iterator;
    using const_iterator = typename storage_t::const_iterator;

    static_vector() noexcept = default;
    explicit static_vector(const usize size)
        noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
      : size_(size)
    {
        std::uninitialized_default_construct(begin(), end());
    }

    static_vector(const usize size, const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
      : size_(size)
    {
        std::uninitialized_fill(begin(), end(), value);
    }

    ~static_vector() noexcept(noexcept(destroy_range(begin(), end()))) { destroy_range(begin(), end()); }

    static_vector(const static_vector&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
    static_vector& operator=(const static_vector&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;

    template<usize OtherCapacity>
        requires (Capacity >= OtherCapacity)
    static_vector(static_vector<T, OtherCapacity>&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : size_(std::exchange(other.size_, 0))
    {
        std::uninitialized_move(other.begin(), other.end(), begin());
    }

    template<usize OtherCapacity>
        requires (Capacity >= OtherCapacity)
    static_vector& operator=(static_vector<T, OtherCapacity>&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        size_ = std::exchange(other.size_, 0);
        std::uninitialized_move(other.begin(), other.end(), begin());
    }

    [[nodiscard]] T& operator[](const usize idx) noexcept { return *ptr(idx); }
    [[nodiscard]] const T& operator[](const usize idx) const noexcept { return *ptr(idx); }
    [[nodiscard]] T* ptr(const usize idx) noexcept { return std::launder(reinterpret_cast<T*>(storage_.ptr(idx))); } // NOLINT
    [[nodiscard]] const T* ptr(const usize idx) const noexcept { return std::launder(reinterpret_cast<const T*>(storage_.ptr(idx))); } // NOLINT
    [[nodiscard]] T& at(const usize idx) noexcept { VK_ASSERT(idx < size_); return *ptr(idx); }
    [[nodiscard]] const T& at(const usize idx) const noexcept { VK_ASSERT(idx < size_); return *ptr(idx); }
    [[nodiscard]] T* data() noexcept { return ptr(0); }
    [[nodiscard]] const T* data() const noexcept { return ptr(0); }

    template<typename... Args>
        requires (std::is_constructible_v<T, Args...>)
    constexpr T& emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        static_assert(std::is_constructible_v<T, Args...>);
        VK_ASSERT(size_ < Capacity);
        std::construct_at(&storage_[size_], std::forward<Args>(args)...);
        return *ptr(size_++);
    }

    constexpr void push_back(const T& t) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        VK_ASSERT(size_ < Capacity);
        std::construct_at(ptr(size_++), t);
    }

    constexpr void push_back(T&& t) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        VK_ASSERT(size_ < Capacity);
        std::construct_at(ptr(size_++), std::move(t));
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] constexpr usize size() const noexcept { return size_; }
    [[nodiscard]] constexpr usize capacity() const noexcept { return Capacity; }

    T& front() noexcept { return at(0); }
    const T& front() const noexcept { return at(0); }
    T& back() noexcept { return at(size() - 1); }
    const T& back() const noexcept { return at(size() - 1); }

    void erase(iterator first, iterator last) noexcept
    {
        std::destroy(first, last);
        std::rotate(first, last, end());
        size_ -= static_cast<usize>(std::distance(first, last));
    }

    void erase(iterator iter) noexcept
    {
        erase(iter, iter + 1);
    }

    constexpr void pop_back() noexcept
    {
        std::destroy(end() - 1, end());
        --size_;
    }

    constexpr void clear() noexcept
    {
        std::destroy(begin(), end());
        size_ = 0;
    }

    constexpr iterator begin() noexcept { return iterator{0}; }
    constexpr iterator end() noexcept { return iterator{size_}; }
    constexpr const_iterator begin() const noexcept { return const_iterator{0}; }
    constexpr const_iterator end() const noexcept { return const_iterator{size_}; }
    constexpr const_iterator cbegin() const noexcept { return const_iterator{0}; }
    constexpr const_iterator cend() const noexcept { return const_iterator{size_}; }
};

} // namespace volkano
