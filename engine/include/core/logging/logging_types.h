/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <string_view>

#include "core/int_types.h"

namespace volkano {

enum class log_verbosity : u8 {
    off, critical, error, warning, info, debug, verbose
};

class log_category {
    std::string_view name_;
    log_verbosity verbosity_;

public:
    log_category(std::string_view name, log_verbosity verbosity);

    void set_verbosity(const log_verbosity v) noexcept { verbosity_ = v; }
    [[nodiscard]] log_verbosity verbosity() const noexcept { return verbosity_; }
    [[nodiscard]] std::string_view name() const noexcept { return name_; }
};

struct log_sink {
    virtual void sink(log_verbosity verbosity, std::string_view log) = 0;
    virtual ~log_sink() = default;
};

} // namespace volkano
