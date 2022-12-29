/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <compare>
#include <type_traits>
#include <initializer_list>

#include "engine/core/int_types.h"
#include "engine/core/assert.h"
#include "engine/core/memory/aligned_union.h"

namespace volkano {

template<typename T, usize Capacity>
    requires (Capacity != 0 && !std::is_void_v<T>)
class static_vector {
    static_assert(Capacity != 0, "Vector capacity must not be zero");

    aligned_union<T> storage_[Capacity];
    usize size_ = 0;

public:
    using value_type = T;
    using size_type = usize;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = typename std::array<T, Capacity>::iterator;
    using const_iterator = typename std::array<T, Capacity>::const_iterator;
    using reverse_iterator = typename std::array<T, Capacity>::reverse_iterator;
    using const_reverse_iterator = typename std::array<T, Capacity>::const_reverse_iterator;

    constexpr static_vector() noexcept = default;
    constexpr explicit static_vector(const usize size)
        noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::default_initializable<T>
      : size_(size)
    {
        std::uninitialized_default_construct(begin(), end());
    }

    constexpr static_vector(const usize size, const T& value)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires std::copy_constructible<T>
      : size_(size)
    {
        std::uninitialized_fill(begin(), end(), value);
    }

    constexpr ~static_vector() noexcept(std::is_nothrow_destructible_v<T>) { std::destroy(begin(), end()); }

    constexpr static_vector(const static_vector& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires std::copyable<T>
      : size_(other.size_)
    {
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }

    constexpr static_vector& operator=(const static_vector& other)
      noexcept(std::is_nothrow_copy_assignable_v<T>)
      requires std::copyable<T>
    {
        if (&other != this) {
            size_ = other.size_;
            std::uninitialized_copy(other.begin(), other.end(), begin());
        }
        return *this;
    }

    constexpr static_vector(static_vector&& other)
        noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::movable<T>
      : size_(std::exchange(other.size_, 0))
    {
        std::uninitialized_move(other.begin(), other.begin() + size_, begin());
    }

    constexpr static_vector& operator=(static_vector&& other)
        noexcept(std::is_nothrow_move_assignable_v<T>)
        requires std::movable<T>
    {
        if (&other != this) {
            size_ = std::exchange(other.size_, 0);
            std::uninitialized_move(other.begin(), other.begin() + size_, begin());
        }
        return *this;
    }

    constexpr static_vector(std::initializer_list<T> elems) noexcept(std::is_nothrow_copy_constructible_v<T>)
      : size_(elems.size())
    {
        VKE_ASSERT(size_ <= Capacity);
        std::uninitialized_copy(elems.begin(), elems.end(), begin());
    };

    constexpr static_vector& operator=(std::initializer_list<T> elems) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        VKE_ASSERT(elems.size() <= Capacity);
        size_ = elems.size();
        std::uninitialized_copy(elems.begin(), elems.end(), begin());
        return *this;
    }

    [[nodiscard]] constexpr T& operator[](const usize idx) noexcept { return *ptr(idx); }
    [[nodiscard]] constexpr const T& operator[](const usize idx) const noexcept { return *ptr(idx); }
    [[nodiscard]] constexpr T* ptr(const usize idx) noexcept { return storage_[idx].template value<T>(); }
    [[nodiscard]] constexpr const T* ptr(const usize idx) const noexcept { return storage_[idx].template value<T>(); }
    [[nodiscard]] constexpr T& at(const usize idx) noexcept { VKE_ASSERT(idx < size_); return *ptr(idx); }
    [[nodiscard]] constexpr const T& at(const usize idx) const noexcept { VKE_ASSERT(idx < size_); return *ptr(idx); }
    [[nodiscard]] constexpr T* data() noexcept { return ptr(0); }
    [[nodiscard]] constexpr const T* data() const noexcept { return ptr(0); }

    template<typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr T& emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        VKE_ASSERT(size_ < Capacity);
        storage_[size_].template construct<T>(std::forward<Args>(args)...);
        return *ptr(size_++);
    }

    constexpr void push_back(const T& t) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        VKE_ASSERT(size_ < Capacity);
        storage_[size_++].template construct<T>(t);
    }

    constexpr void push_back(T&& t) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        VKE_ASSERT(size_ < Capacity);
        storage_[size_++].template construct<T>(std::move(t));
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] constexpr usize size() const noexcept { return size_; }
    [[nodiscard]] constexpr usize max_size() const noexcept { return Capacity; }
    [[nodiscard]] constexpr usize capacity() const noexcept { return Capacity; }

    constexpr T& front() noexcept { return at(0); }
    constexpr const T& front() const noexcept { return at(0); }
    constexpr T& back() noexcept { return at(size() - 1); }
    constexpr const T& back() const noexcept { return at(size() - 1); }

    constexpr void erase(iterator first, iterator last) noexcept
    {
        std::destroy(first, last);
        std::rotate(first, last, end());
        size_ -= static_cast<usize>(std::distance(first, last));
    }

    constexpr void erase(iterator iter) noexcept
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

    constexpr iterator begin() noexcept { return iterator{ptr(0)}; }
    constexpr const_iterator begin() const noexcept { return const_iterator{ptr(0)}; }
    constexpr iterator end() noexcept { return iterator{ptr(0), size_}; }
    constexpr const_iterator end() const noexcept { return const_iterator{ptr(0), size_}; }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{begin()}; }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator{end()}; }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
    constexpr const_iterator cbegin() const noexcept { return const_iterator{ptr(0)}; }
    constexpr const_iterator cend() const noexcept { return const_iterator{ptr(0), size_}; }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

    constexpr void swap(static_vector& other) noexcept(std::is_nothrow_swappable_v<T>)
    {
        using std::swap;
        swap(storage_, other.storage_);
        swap(size_, other.size_);
    }
};

template<typename T, usize Capacity>
    requires std::equality_comparable<T>
bool operator==(const static_vector<T, Capacity>& left, const static_vector<T, Capacity>& right)
{
    if (left.size() != right.size()) {
        return false;
    }
    return std::equal(left.begin(), left.end(), right.begin(), right.end());
}

template<typename T, usize Capacity>
    requires std::three_way_comparable<T>
auto operator<=>(const static_vector<T, Capacity>& left, const static_vector<T, Capacity>& right)
{
    return std::lexicographical_compare_three_way(left.begin(), left.end(), right.begin(), right.end());
}

template<typename T, usize Capacity>
constexpr void swap(static_vector<T, Capacity>& left, static_vector<T, Capacity>& right) noexcept(std::is_nothrow_swappable_v<T>)
{
    left.swap(right);
}

} // namespace volkano
