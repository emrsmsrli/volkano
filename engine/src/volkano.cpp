/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "engine/volkano.h"
#include "engine/core/util/variant_visit_nt.h"
#include "engine/renderer/vk_renderer.h"

VKE_DEFINE_LOG_CATEGORY_STATIC(engine, verbose);

namespace volkano {

engine::engine() noexcept
    : engine(std::make_unique<vk_renderer>(this))
{
}

engine::engine(std::unique_ptr<renderer_interface> renderer) noexcept
  : renderer_{std::move(renderer)}
{
    VKE_ASSERT(renderer_ != nullptr);

    VKE_ASSERT_MSG(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) >= 0, "SDL init error: {}", SDL_GetError());

    // todo read from config, dont init right away
    window_ = SDL_CreateWindow("volkano",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      1280, 720,
      SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    VKE_ASSERT_MSG(window_, "SDL window create error: {}", SDL_GetError());
    renderer_->initialize();
}

engine::~engine() noexcept
{
    SDL_Quit();
}

bool engine::tick() noexcept
{
    while (SDL_PollEvent(&window_event_)) {
        switch (window_event_.type) {
            case SDL_QUIT:
                return false;
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                break;
            case SDL_WINDOWEVENT: {
                switch (window_event_.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        renderer_->on_window_resize();
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        should_render_ = false;
                        break;
                    case SDL_WINDOWEVENT_RESTORED:
                        should_render_ = true;
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    if (should_render_) {
        renderer_->render();
    }
    return true;
}

vec2u engine::get_window_extent() noexcept
{
    vec2u window_extent;
    SDL_Vulkan_GetDrawableSize(window_,
      reinterpret_cast<int*>(&window_extent.x),
      reinterpret_cast<int*>(&window_extent.y));
    return window_extent;
}

} // namespace volkano
