/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <ranges>
#include <utility>

namespace volkano::algo {

template<typename Iter, typename Elem, typename Projection = std::identity>
decltype(auto) find_ptr(Iter begin, Iter end, const Elem& elem, const Projection& proj = {}) noexcept
{
    const auto it = std::ranges::find(begin, end, elem, proj);
    return it == end ? nullptr : &*it;
}

template<typename Range, typename Elem, typename Projection = std::identity>
decltype(auto) find_ptr(Range&& range, const Elem& elem, const Projection& proj = {}) noexcept
{
    const auto it = std::ranges::find(std::forward<Range>(range), elem, proj);
    return it == range.end() ? nullptr : &*it;
}

template<typename Iter, typename Pred, typename Projection = std::identity>
decltype(auto) find_if_ptr(Iter begin, Iter end, Pred pred, const Projection& proj = {}) noexcept
{
    const auto it = std::ranges::find_if(begin, end, pred, proj);
    return it == end ? nullptr : &*it;
}

template<typename Range, typename Pred, typename Projection = std::identity>
decltype(auto) find_if_ptr(Range&& range, Pred pred, const Projection& proj = {}) noexcept
{
    const auto it = std::ranges::find_if(std::forward<Range>(range), pred, proj);
    return it == range.end() ? nullptr : &*it;
}

} // namespace volkano::algo
