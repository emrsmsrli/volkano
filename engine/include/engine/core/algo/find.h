/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <utility>

namespace volkano::algo {

template<typename Range, typename Elem>
auto find(Range&& range, Elem&& elem) noexcept
{
    return std::find(range.begin(), range.end(), std::forward<Elem>(elem));
}

template<typename Range, typename Pred>
auto find_if(Range&& range, Pred&& pred) noexcept
{
    return std::find_if(range.begin(), range.end(), std::forward<Pred>(pred));
}

template<typename Iter, typename Elem>
decltype(auto) find_ptr(Iter begin, Iter end, Elem&& elem) noexcept
{
    const auto it = std::find(begin, end, std::forward<Elem>(elem));
    return it == end ? nullptr : &*it;
}

template<typename Range, typename Pred>
decltype(auto) find_ptr(Range&& range, Pred&& pred) noexcept
{
    return find_ptr(range.begin(), range.end(), std::forward<Pred>(pred));
}

template<typename Iter, typename Pred>
decltype(auto) find_if_ptr(Iter begin, Iter end, Pred&& pred) noexcept
{
    const auto it = std::find_if(begin, end, std::forward<Pred>(pred));
    return it == end ? nullptr : &*it;
}

template<typename Range, typename Pred>
decltype(auto) find_if_ptr(Range&& range, Pred&& pred) noexcept
{
    return find_if_ptr(range.begin(), range.end(), std::forward<Pred>(pred));
}

} // namespace volkano::algo
