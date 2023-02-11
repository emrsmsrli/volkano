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
#include <range/v3/iterator/access.hpp>

namespace volkano::string {

struct split_params {
    std::string_view src;
    std::string_view delims;
    bool allow_empty_tokens = false;
};

template<typename T>
concept string_like =
  std::same_as<T, std::string> ||
  std::same_as<T, std::string_view> ||
  std::same_as<T, const char*>;

template<typename T>
  requires string_like<T>
struct join_params {
    std::span<const T> src;
    std::string_view delim = ", ";
    std::string_view begin;
    std::string_view end;

    explicit join_params(std::span<const T> src,
      std::string_view delim = ", ",
      std::string_view begin = "",
      std::string_view end = "")
      : src(src), delim(delim), begin(begin), end(end) {}
};

template<typename Rng, typename... Args>
    requires ranges::range<Rng>
join_params(Rng&&, Args&&...) -> join_params<ranges::iter_value_t<Rng>>;

std::vector<std::string_view> split(const split_params& params) noexcept;
std::vector<std::string_view> split_lines(std::string_view src, bool allow_empty_tokens = false) noexcept;

template<typename T>
std::string join(const join_params<T>& params) noexcept
{
    if (params.src.empty()) {
        return {};
    }

    usize joined_size = ranges::accumulate(params.src, 0, [](usize size, const auto& elem) {
        if constexpr (std::is_same_v<const char*, std::remove_cvref_t<decltype(elem)>>) {
            return size + std::strlen(elem);
        } else {
            return size + elem.size();
        }
    });
    joined_size += params.delim.size() * (params.src.size() - 1);
    joined_size += params.begin.size() + params.end.size();

    std::string joined;
    joined.reserve(joined_size);

    if (params.begin.data()) {
        joined.append(params.begin);
    }

    joined.append(params.src[0]);
    for (usize i = 1; i < params.src.size(); ++i) {
        joined.append(params.delim);
        joined.append(params.src[i]);
    }

    if (params.end.data()) {
        joined.append(params.end);
    }

    return joined;
}

template<typename Rng>
    requires ranges::range<Rng>
std::string join(Rng&& range) noexcept
{
    return join(join_params<ranges::iter_value_t<Rng>>{range});
}

} // namespace volkano::string
