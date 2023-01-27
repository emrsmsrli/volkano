/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <ranges>
#include <utility>

#include "core/int_types.h"

namespace volkano::algo {

template<typename Iter, typename Elem, typename Projection = std::identity>
ptrdiff index_of(Iter begin, Iter end, const Elem& elem, const Projection& proj = {}) noexcept
{
    const auto iter = std::ranges::find(begin, end, elem, proj);
    return iter == end ? ptrdiff{-1} : std::ranges::distance(begin, iter);
}

template<typename Range, typename Elem, typename Projection = std::identity>
ptrdiff index_of(Range&& range, const Elem& elem, const Projection& proj = {}) noexcept
{
    const auto iter = std::ranges::find(std::forward<Range>(range), elem, proj);
    return iter == range.end()? ptrdiff{-1} : std::ranges::distance(range.begin(), iter);
}

template<typename Iter, typename Pred, typename Projection = std::identity>
ptrdiff index_of_by_predicate(Iter begin, Iter end, Pred pred, const Projection& proj = {}) noexcept
{
    const auto iter = std::ranges::find_if(begin, end, pred, proj);
    return iter == end ? ptrdiff{-1} : std::ranges::distance(begin, iter);
}

template<typename Range, typename Pred, typename Projection = std::identity>
ptrdiff index_of_by_predicate(Range&& range, Pred pred, const Projection& proj = {}) noexcept
{
    const auto iter = std::ranges::find_if(std::forward<Range>(range), pred, proj);
    return iter == range.end() ? ptrdiff{-1} : std::ranges::distance(range.begin(), iter);
}

} // namespace volkano::algo
