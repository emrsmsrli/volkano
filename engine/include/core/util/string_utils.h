/*
 * Copyright (C) 2022  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <concepts>
#include <string_view>
#include <span>
#include <vector>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/range/operations.hpp>
#include <range/v3/iterator/access.hpp>

namespace volkano::string {

template<typename T>
concept string_like =
  std::same_as<T, std::string> ||
  std::same_as<T, std::string_view> ||
  std::same_as<T, const char*>;

template<typename Rng>
concept string_like_range = ranges::range<Rng> && string_like<ranges::iter_value_t<Rng>>;


template<string_like_range Delims>
u32 split_into(std::vector<std::string_view>& out, std::string_view src, Delims&& delims, bool cull_empty = true) noexcept;
u32 split_into(std::vector<std::string_view>& out, std::string_view src, std::string_view delim = ",", bool cull_empty = true) noexcept;

template<string_like_range Delims>
std::vector<std::string_view> split(std::string_view src, Delims&& delims, bool cull_empty = true) noexcept;
std::vector<std::string_view> split(std::string_view src, std::string_view delim = ",", bool cull_empty = true) noexcept;
std::vector<std::string_view> split_lines(std::string_view src, bool cull_empty = true) noexcept;

template<string_like_range Source>
std::string join(Source&& src, std::string_view delim = ", ", std::string_view begin = "", std::string_view end = "") noexcept;


template<string_like_range Delims>
u32 split_into(std::vector<std::string_view>& out, std::string_view src, Delims&& delims, bool cull_empty) noexcept
{
    if (src.empty() || std::empty(delims)) {
        return 0;
    }

    out.clear();

    const auto src_end = src.end();
    auto last_it = src.begin();

    const auto try_emplace = [&](auto end_it) {
        if (!cull_empty || ranges::distance(last_it, end_it) > 0) {
            out.emplace_back(last_it, end_it);
        }
    };

    const auto find_first_delim = [&]() {
        const std::string_view src_substr{last_it, src_end};
        auto min_it = src_end;
        for (const std::string_view d : delims) {
            auto found_idx = src_substr.find(d);
            if (found_idx != std::string_view::npos) {
                if (auto found_it = last_it + static_cast<ptrdiff>(found_idx);
                  found_it < min_it) {
                    min_it = found_it;
                }
            }
        }
        return min_it;
    };

    while (last_it != src_end) {
        const auto delim_it = find_first_delim();

        if (delim_it == src_end) {
            break;
        }

        try_emplace(delim_it);
        last_it = delim_it + 1;
    }

    try_emplace(src_end);
    return static_cast<u32>(out.size());
}

template<string_like_range Delims>
std::vector<std::string_view> split(std::string_view src, Delims&& delims, bool cull_empty) noexcept
{
    std::vector<std::string_view> tokens;
    split_into(tokens, src, std::forward<Delims>(delims), cull_empty);
    return tokens;
}

template<string_like_range Source>
std::string join(Source&& src, std::string_view delim, std::string_view begin, std::string_view end) noexcept
{
    if (std::empty(src)) {
        return {};
    }

    usize joined_size = ranges::accumulate(src, 0, [](usize size, const auto& elem) {
        if constexpr (std::is_same_v<const char*, ranges::iter_value_t<Source>>) {
            return size + std::strlen(elem);
        } else {
            return size + elem.size();
        }
    });

    const usize src_size = std::size(src);

    joined_size += delim.size() * (src_size - 1);
    joined_size += begin.size() + end.size();

    std::string joined;
    joined.reserve(joined_size);

    if (begin.data()) {
        joined.append(begin);
    }

    joined.append(src[0]);
    for (usize i = 1; i < src_size; ++i) {
        joined.append(delim);
        joined.append(src[i]);
    }

    if (end.data()) {
        joined.append(end);
    }

    return joined;
}

} // namespace volkano::string
