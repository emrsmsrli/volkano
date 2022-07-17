/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <utility>

namespace volkano::algo {

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
bool contains_if(Iter begin, Iter end, Pred&& pred) noexcept
{
    return std::find_if(begin, end, std::forward<Pred>(pred)) != end;
}

template<typename T, typename Pred>
bool contains_if(T&& range, Pred&& pred) noexcept
{
    return contains_if(range.begin(), range.end(), std::forward<Pred>(pred));
}

} // namespace volkano::algo
