/*
 * Copyright (C) 2022  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "core/util/string_utils.h"

#include <range/v3/algorithm/contains.hpp>
#include <range/v3/view/enumerate.hpp>

namespace volkano::string {

std::vector<std::string_view> split(const std::string_view src, const std::string_view delims /*= ","*/) noexcept
{
    std::vector<std::string_view> tokens;
    if (src.empty() || delims.empty()) {
        return tokens;
    }

    usize last_idx = 0;

    for (const auto [idx, ch] : ranges::views::enumerate(src)) {
        if (ranges::contains(delims, ch)) {
            const std::string_view token{src.begin() + last_idx, src.begin() + idx};
            if (last_idx != idx) {
                tokens.emplace_back(token);
            }
            last_idx = idx + 1;
        }
    }

    if (last_idx != src.size()) {
        tokens.emplace_back(src.begin() + last_idx, src.end());
    }
    return tokens;
}

std::vector<std::string_view> split_lines(const std::string_view src) noexcept
{
    return split(src, "\r\n");
}

} // namespace volkano::string
