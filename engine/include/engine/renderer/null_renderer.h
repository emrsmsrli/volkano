/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "engine/renderer/renderer_interface.h"

namespace volkano {

class null_renderer : public renderer_interface {
public:
    void initialize() noexcept override {}
    void on_window_resize() noexcept override {}
    void render() noexcept override {}
};

} // namespace volkano
