/*
 * Copyright (C) 2022  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "core/util/string_utils.h"

namespace volkano::string {

u32 split_into(std::vector<std::string_view>& out, std::string_view src,
  std::string_view delim /*=","*/, bool cull_empty /*=true*/) noexcept
{
    const std::string_view delims[]{delim};
    return split_into(out, src, delims, cull_empty);
}

std::vector<std::string_view> string::split(std::string_view src,
  std::string_view delim /*=","*/, bool cull_empty /*=true*/) noexcept
{
    std::vector<std::string_view> tokens;
    const std::string_view delims[]{delim};
    split_into(tokens, src, delims, cull_empty);
    return tokens;
}

std::vector<std::string_view> split_lines(std::string_view src, bool cull_empty /*=true*/) noexcept
{
    static constexpr std::string_view line_delims[] = {"\r\n", "\n", "\r"};
    return split(src, line_delims, cull_empty);
}

} // namespace volkano::string
