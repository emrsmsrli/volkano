/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifndef VOLKANO_CONTAINER_UTILS_H
#define VOLKANO_CONTAINER_UTILS_H

#include <algorithm>
#include <utility>

namespace volkano::range_utils {

template<typename Iter, typename Elem>
bool contains(Iter begin, Iter end, Elem&& elem) noexcept
{
    return std::find(begin, end, std::forward<Elem>(elem)) != end;
}

template<typename Range, typename Elem>
bool contains(Range&& range, Elem&& elem) noexcept
{
    return contains(range.begin(), range.end(), std::forward<Elem>(elem));
}

template<typename Iter, typename Pred>
bool contains_by_predicate(Iter begin, Iter end, Pred&& pred) noexcept
{
    return std::find_if(begin, end, std::forward<Pred>(pred)) != end;
}

template<typename T, typename Pred>
bool contains_by_predicate(T&& range, Pred&& pred) noexcept
{
    return contains_by_predicate(range.begin(), range.end(), std::forward<Pred>(pred));
}

} // namespace volkano::ranges

#endif // VOLKANO_CONTAINER_UTILS_H
