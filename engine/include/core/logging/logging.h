/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <string_view>
#include <memory>
#include <source_location>
#include <vector>

#include <fmt/format.h>

#include "core/int_types.h"
#include "core/platform.h"
#include "logging_types.h"

#ifndef VKE_LOG_COMPILE_TIME_VERBOSITY
  #define VKE_LOG_COMPILE_TIME_VERBOSITY ::volkano::log_verbosity::warning
#endif // VKE_LOG_COMPILE_TIME_VERBOSITY

#define VKE_DECLARE_LOG_CATEGORY(name) extern ::volkano::log_category logcat_ ## name
#define VKE_DEFINE_LOG_CATEGORY(name, default_verbosity) ::volkano::log_category logcat_ ## name{#name, ::volkano::log_verbosity::default_verbosity} /*NOLINT cert-err58-cpp*/
#define VKE_DEFINE_LOG_CATEGORY_STATIC(name, default_verbosity) static VKE_DEFINE_LOG_CATEGORY(name, default_verbosity)

#define VKE_LOG(category, verbosity, format, ...)                                        \
do {                                                                                     \
    constexpr auto v_current = ::volkano::log_verbosity::verbosity;                      \
    constexpr auto v_allowed = ::volkano::log_verbosity{VKE_LOG_COMPILE_TIME_VERBOSITY}; \
    if constexpr (v_current <= v_allowed) {                                              \
        ::volkano::logger::get().log(logcat_ ## category,                                \
          v_current, std::source_location::current(),                                    \
          format __VA_OPT__(,) __VA_ARGS__);                                             \
    }                                                                                    \
} while(0)

#define VKE_CLOG(condition, category, verbosity, format, ...) do { if((condition)) { VKE_LOG(category, verbosity, format, __VA_ARGS__); } } while(0)

#define VKE_LOG_DYN(category, verbosity, format, ...)                                    \
do {                                                                                     \
    constexpr auto v_allowed = ::volkano::log_verbosity{VKE_LOG_COMPILE_TIME_VERBOSITY}; \
    if (verbosity <= v_allowed) {                                                        \
        ::volkano::logger::get().log(logcat_ ## category,                                \
          verbosity, std::source_location::current(),                                    \
          format __VA_OPT__(,) __VA_ARGS__);                                             \
    }                                                                                    \
} while(0)

#define VKE_CLOG_DYN(condition, category, verbosity, format, ...) do { if((condition)) { VKE_LOG_DYN(category, verbosity, format, __VA_ARGS__); } } while(0)

namespace volkano {

class logger {
    friend log_category;

    std::vector<log_category*> categories_;
    std::vector<std::unique_ptr<log_sink>> sinks_;

    using log_buffer = fmt::basic_memory_buffer<char, 1024>;

public:
    static logger& get() noexcept;

    template<typename Fmt, typename... Args>
    void log(const log_category& category, const log_verbosity verbosity,
      const std::source_location src, const Fmt& fmt, Args&& ... args) noexcept
    {
        if (category.verbosity() < verbosity) {
            return;
        }

        log_buffer buffer;
        fmt::vformat_to(std::back_inserter(buffer), fmt, fmt::make_format_args(args...));
        const std::string_view user_log{buffer.data(), buffer.size()};

        log_internal(buffer, category, verbosity, src, user_log);
    }

    void set_category_verbosity(std::string_view category_name, log_verbosity verbosity) noexcept;

private:
    logger();

    void log_internal(log_buffer& buffer, const log_category& category, log_verbosity verbosity,
      std::source_location src, std::string_view user_log) noexcept;

    void register_log_category(log_category* category);

    log_category* find_category_by_name(std::string_view name) noexcept;
};

} // namespace volkano

VKE_DECLARE_LOG_CATEGORY(general);
