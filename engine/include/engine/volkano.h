/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <memory>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>

#include "engine/core/int_types.h"
#include "engine/core/math/vec2.h"
#include "engine/renderer/renderer_interface.h"

namespace volkano {

class engine {
    std::unique_ptr<renderer_interface> renderer_;
    SDL_Window* window_ =  nullptr;
    SDL_Event window_event_{};

    bool should_render_ = true;

public:
    engine() noexcept;
    explicit engine(std::unique_ptr<renderer_interface> renderer) noexcept;
    ~engine() noexcept;

    engine(const engine&) = delete;
    engine(engine&) = delete;
    engine& operator=(const engine&) = delete;
    engine& operator=(engine&&) = delete;

    bool tick() noexcept;

    SDL_Window* get_window() noexcept { return window_; }
    vec2u get_window_extent() noexcept;
};

} // namespace volkano
