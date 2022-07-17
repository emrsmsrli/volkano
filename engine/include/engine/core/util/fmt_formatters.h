/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <type_traits>
#include <optional>
#include <string_view>

#include <fmt/core.h>
#include <fmt/chrono.h>
#include <magic_enum.hpp>

template<typename T>
    requires std::is_enum_v<T>
struct fmt::formatter<T> : formatter<std::string_view> {
    template<typename FormatContext>
    auto format(const T e, FormatContext& ctx) -> decltype(ctx.out())
    {
        return formatter<std::string_view>::format(magic_enum::enum_name(e), ctx);
    }
};

template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T> {
    template<typename FormatContext>
    auto format(const std::optional<T>& opt, FormatContext& ctx) -> decltype(ctx.out())
    {
        if (!opt.has_value()) {
            return formatter<std::string_view>{}.format("empty", ctx);
        }

        return formatter<T>::format(opt.value(), ctx);
    }
};
