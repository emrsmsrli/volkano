/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <utility>

#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/find_if.hpp>

namespace volkano::algo {

template<typename Iter, typename Elem, typename Projection = std::identity>
bool contains(Iter begin, Iter end, const Elem& elem, const Projection& proj = {}) noexcept
{
    return ranges::find(begin, end, elem, proj) != end;
}

template<typename Range, typename Elem, typename Projection = std::identity>
bool contains(Range&& range, const Elem& elem, const Projection& proj = {}) noexcept
{
    return ranges::find(std::forward<Range>(range), elem, proj) != range.end();
}

template<typename Iter, typename Pred, typename Projection = std::identity>
bool contains_if(Iter begin, Iter end, Pred pred, const Projection& proj = {}) noexcept
{
    return ranges::find_if(begin, end, pred, proj) != end;
}

template<typename Range, typename Pred, typename Projection = std::identity>
bool contains_if(Range&& range, Pred pred, const Projection& proj = {}) noexcept
{
    return ranges::find_if(std::forward<Range>(range), pred, proj) != range.end();
}

} // namespace volkano::algo
