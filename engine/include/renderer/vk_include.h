/*
 * Copyright (C) 2022  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#define VK_NO_PROTOTYPES
#define VULKAN_HPP_ASSERT_ON_RESULT(...)
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_SETTERS
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_SMART_HANDLE
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <vulkan/vulkan.hpp>
#include <vulkan_mem_alloc/alloc.hpp>
#include <range/v3/algorithm/contains.hpp>

#include "core/assert.h"
#include "renderer/vk_fmt_formatters.h"

namespace volkano {

inline void vk_check_result(vk::Result result)
{
    VKE_ASSERT_MSG(result == vk::Result::eSuccess, "result: {}", result);
}

inline void vk_check_result(vk::Result result, std::span<vk::Result> invalid_results)
{
    VKE_ASSERT_MSG(!ranges::contains(invalid_results, result), "result: {}", result);
}

[[nodiscard]] decltype(auto) vk_check_result(auto vk_ret)
{
    vk_check_result(vk_ret.result);
    return vk_ret.value;
}

} // namespace volkano

