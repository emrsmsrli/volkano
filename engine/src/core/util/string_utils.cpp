/*
 * Copyright (C) 2022  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "core/util/string_utils.h"

#include <range/v3/algorithm/find_first_of.hpp>
#include <range/v3/distance.hpp>

namespace volkano::string {

u32 split_into(std::vector<std::string_view>& tokens, const split_params& params) noexcept
{
    if (params.src.empty() || params.delims.empty()) {
        return 0;
    }

    tokens.clear();

    const auto src_end = params.src.end();
    auto last_it = params.src.begin();

    while (last_it != src_end) {
        const auto delim_it = ranges::find_first_of(last_it, src_end, params.delims.begin(), params.delims.end());
        if (delim_it == src_end) {
            break;
        }

        if (!params.cull_empty || ranges::distance(last_it, delim_it) > 0) {
            tokens.emplace_back(last_it, delim_it);
        }

        last_it = delim_it + 1;
    }

    if (!params.cull_empty || ranges::distance(last_it, src_end) > 0) {
        tokens.emplace_back(last_it, src_end);
    }

    return tokens.size();
}

std::vector<std::string_view> string::split(const split_params& params) noexcept
{
    std::vector<std::string_view> tokens;
    split_into(tokens, params);
    return tokens;
}

std::vector<std::string_view> split(const std::string_view src, const std::string_view delims /*= ","*/) noexcept
{
    return split({src, delims});
}

std::vector<std::string_view> split_lines(const std::string_view src) noexcept
{
    return split({src, "\r\n"});
}

} // namespace volkano::string
