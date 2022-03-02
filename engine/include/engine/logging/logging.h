/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifndef VOLKANO_LOG_CATEGORY_H
#define VOLKANO_LOG_CATEGORY_H

#include <string_view>
#include <memory>
#include <vector>

#include <fmt/format.h>

#include "engine/types.h"
#include "engine/core/platform.h"
#include "engine/logging/logging_types.h"
#include "engine/logging/source_loc.h"

#ifndef VK_LOG_COMPILE_TIME_VERBOSITY
#define VK_LOG_COMPILE_TIME_VERBOSITY ::volkano::log_verbosity::warning
#endif // VK_LOG_COMPILE_TIME_VERBOSITY

#define VK_DECLARE_LOG_CATEGORY(name) extern ::volkano::log_category logcat_ ## name
#define VK_DEFINE_LOG_CATEGORY(name, default_verbosity) ::volkano::log_category logcat_ ## name{#name, ::volkano::log_verbosity::default_verbosity} /*NOLINT cert-err58-cpp*/
#define VK_DEFINE_LOG_CATEGORY_STATIC(name, default_verbosity) static VK_DEFINE_LOG_CATEGORY(name, default_verbosity)

// todo replace with std::source_location
#define VK_LOG(category, verbosity, fmt, ...) do {                                      \
    constexpr auto v_current = ::volkano::log_verbosity::verbosity;                     \
    constexpr auto v_allowed = ::volkano::log_verbosity{VK_LOG_COMPILE_TIME_VERBOSITY}; \
    if constexpr(v_current <= v_allowed) {                                              \
        ::volkano::logger::get().log(logcat_ ## category,                               \
            v_current, {__FILE__, __LINE__}, fmt __VA_OPT__(,) __VA_ARGS__);            \
    }                                                                                   \
} while(0)

#define VK_CLOG(condition, category, verbosity, fmt, ...) do { if((condition)) { VK_LOG(category, verbosity, fmt, __VA_ARGS__); } } while(0)

namespace volkano {

class logger {
    friend log_category;

    std::vector<log_category*> categories_;
    std::vector<std::unique_ptr<log_sink>> sinks_;

    using log_buffer = fmt::basic_memory_buffer<char, 1024>;

public:
    static logger& get() noexcept;

    template<typename Char, typename... Args>
    void log(const log_category& category, const log_verbosity verbosity, const source_loc src,
             const Char* fmt, Args&&... args) noexcept
    {
        if(category.verbosity() < verbosity) {
            return;
        }

        log_buffer buffer;
        const auto fmt_args = fmt::make_format_args(args...);
        fmt::vformat_to(std::back_inserter(buffer), fmt, fmt_args);
        const std::string_view user_log{buffer.data(), buffer.size()};

        log_internal(buffer, category, verbosity, src, user_log);
    }

    void set_category_verbosity(std::string_view category_name, log_verbosity verbosity);

private:
    logger();

    void log_internal(log_buffer& buffer, const log_category& category,
                      log_verbosity verbosity, source_loc src, std::string_view user_log) noexcept;

    void register_log_category(log_category* category);
    log_category* find_category_by_name(std::string_view name) noexcept;
};

} // namespace volkano

VK_DECLARE_LOG_CATEGORY(general);

#endif // VOLKANO_LOG_CATEGORY_H
