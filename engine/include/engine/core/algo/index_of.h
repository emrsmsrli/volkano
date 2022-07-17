/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <utility>

#include "engine/core/int_types.h"

namespace volkano::algo {

template<typename Iter, typename Elem>
ptrdiff index_of(Iter begin, Iter end, Elem&& elem) noexcept
{
    const auto iter = std::find(begin, end, std::forward<Elem>(elem));
    return iter == end ? ptrdiff{-1} : std::distance(begin, iter);
}

template<typename Range, typename Elem>
ptrdiff index_of(Range&& range, Elem&& elem) noexcept
{
    return index_of(range.begin(), range.end(), std::forward<Elem>(elem));
}

template<typename Iter, typename Pred>
ptrdiff index_of_by_predicate(Iter begin, Iter end, Pred&& pred) noexcept
{
    const auto iter = std::find_if(begin, end, std::forward<Pred>(pred));
    return iter == end ? ptrdiff{-1} : std::distance(begin, iter);
}

template<typename Range, typename Pred>
ptrdiff index_of_by_predicate(Range&& range, Pred&& pred) noexcept
{
    return index_of_by_predicate(range.begin(), range.end(), std::forward<Pred>(pred));
}

} // namespace volkano::algo
