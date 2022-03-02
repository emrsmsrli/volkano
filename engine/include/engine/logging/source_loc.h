/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifndef VOLKANO_SOURCE_LOC_H
#define VOLKANO_SOURCE_LOC_H

#include <string_view>

#include <fmt/core.h>

#include "engine/types.h"

namespace volkano {

struct source_loc {
    std::string_view file;
    u32 line;
};

} // namespace volkano

template<>
struct fmt::formatter<volkano::source_loc> : public formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const volkano::source_loc& loc, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}:{}", loc.file, loc.line);
    }
};

#endif // VOLKANO_SOURCE_LOC_H
