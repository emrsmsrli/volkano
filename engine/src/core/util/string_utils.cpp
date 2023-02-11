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

std::vector<std::string_view> split(const split_params& params) noexcept
{
    std::vector<std::string_view> tokens;
    if (params.src.empty() || params.delims.empty()) {
        return tokens;
    }

    usize last_idx = 0;

    for (const auto [idx, ch] : ranges::views::enumerate(params.src)) {
        if (ranges::contains(params.delims, ch)) {
            const std::string_view token{params.src.begin() + last_idx, params.src.begin() + idx};
            if (!token.empty() || params.allow_empty_tokens) {
                tokens.emplace_back(token);
                last_idx = idx + 1;
            }
        }
    }

    if (last_idx != params.src.size() || params.allow_empty_tokens) {
        tokens.emplace_back(params.src.begin() + last_idx, params.src.end());
    }
    return tokens;
}

std::vector<std::string_view> split_lines(const std::string_view src, const bool allow_empty_tokens) noexcept
{
    return split(split_params{src, "\n", allow_empty_tokens});
}

} // namespace volkano::string
