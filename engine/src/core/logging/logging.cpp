/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "engine/core/logging/logging.h"

#include <thread>

#include <fmt/os.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include "engine/core/util/fmt_formatters.h"
#include "engine/core/util/ranges.h"

VK_DEFINE_LOG_CATEGORY(general, info);

namespace volkano {

namespace {

struct default_file_sink : log_sink {
    fmt::file log_file{"log.txt", fmt::file::RDWR | fmt::file::CREATE};

    void sink(const log_verbosity /*verbosity*/, const std::string_view log) override
    {
        log_file.write(log.data(), log.size());
    }
};

struct default_stdout_sink : log_sink {
    void sink(const log_verbosity verbosity, const std::string_view log) override
    {
        fmt::print(verbosity_style(verbosity), "{}", log);
    }

    static fmt::text_style verbosity_style(const log_verbosity verbosity)
    {
        switch (verbosity) {
            case log_verbosity::verbose:
                return fmt::emphasis::italic | fmt::fg(fmt::color::dim_gray);
            case log_verbosity::debug:
                return fmt::fg(fmt::color::gray);
            case log_verbosity::warning:
                return fmt::fg(fmt::color::yellow);
            case log_verbosity::error:
                return fmt::fg(fmt::color::red);
            case log_verbosity::critical:
                return fmt::emphasis::bold | fmt::bg(fmt::color::dark_red);
            case log_verbosity::info:
            case log_verbosity::off:
            default:
                return fmt::text_style{};
        }
    }
};

auto verbosity_style(const log_verbosity verbosity)
{
    switch (verbosity) {
        case log_verbosity::off:
        case log_verbosity::critical:
        case log_verbosity::error:
        case log_verbosity::warning:
        case log_verbosity::info:
        case log_verbosity::debug:
        case log_verbosity::verbose:
        default:
            return fmt::fg(fmt::color::white);
    }
}

}

log_category::log_category(const std::string_view name, const log_verbosity verbosity)
  : name_(name),
    verbosity_(verbosity)
{
    logger::get().register_log_category(this);
}

logger& logger::get() noexcept
{
    static logger instance;
    return instance;
}

void logger::register_log_category(log_category* category)
{
    categories_.push_back(category);
}

void logger::set_category_verbosity(const std::string_view category_name, const log_verbosity verbosity) noexcept
{
    if (log_category* cat = find_category_by_name(category_name)) {
        cat->set_verbosity(verbosity);
    }
}

log_category* logger::find_category_by_name(const std::string_view name) noexcept
{
    const auto it = find_if(categories_, [name](const log_category* cat) { return name == cat->name(); });
    return it == categories_.end() ? nullptr : *it;
}

logger::logger()
{
    sinks_.push_back(std::make_unique<default_file_sink>());
    sinks_.push_back(std::make_unique<default_stdout_sink>());
}

void logger::log_internal(logger::log_buffer& buffer, const log_category& category, const log_verbosity verbosity,
  const std::source_location src, const std::string_view user_log) noexcept
{
#if PLATFORM_WINDOWS
    constexpr const char path_separator = '\\';
#elif PLATFORM_UNIX
    constexpr const char path_separator = '/';
#endif

    std::string_view file_name = src.file_name();
    const auto file_start = file_name.find_last_of(path_separator);
    if (file_start != std::string_view::npos) {
        file_name = file_name.substr(file_start + 1);
    }

    const auto now = std::chrono::system_clock::now();
    fmt::format_to(std::back_inserter(buffer), "[{:%H:%M}:{:%S}][{}][{}:{}][{}][{}]: {}\n",
      now, now.time_since_epoch(), std::this_thread::get_id(),
      file_name, src.line(), category.name(), verbosity, user_log);

    const std::string_view log{buffer.begin() + user_log.size(), buffer.size() - user_log.size()};
    for (const auto& sink: sinks_) {
        sink->sink(verbosity, log);
    }
}

} // namespace volkano
