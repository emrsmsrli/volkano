/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <utility>

#include <range/v3/algorithm/find_if.hpp>

namespace volkano::algo {

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
