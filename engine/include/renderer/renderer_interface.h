/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace volkano {

class renderer_interface {
public:
    virtual ~renderer_interface() = default;
    virtual void initialize() noexcept = 0;
    virtual void on_window_resize() noexcept = 0;
    virtual void render() noexcept = 0;
};

} // namespace volkano
