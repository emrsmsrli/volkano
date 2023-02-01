/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#define VULKAN_HPP_ASSERT_ON_RESULT(...)
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_SETTERS
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <vulkan_mem_alloc/alloc.hpp>
#include <fmt/format.h>
#include <range/v3/algorithm/contains.hpp>

#include "core/assert.h"

template<>
struct fmt::formatter<vk::ExtensionProperties> : formatter<std::string>
{
    template <typename FormatContext>
    auto format(const vk::ExtensionProperties& ext, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}({})", static_cast<std::string_view>(ext.extensionName), ext.specVersion);
    }
};

template<>
struct fmt::formatter<vk::LayerProperties> : formatter<std::string>
{
    template <typename FormatContext>
    auto format(const vk::LayerProperties& layer, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}(spec {} impl {})",
          static_cast<std::string_view>(layer.layerName),
          layer.specVersion, layer.implementationVersion);
    }
};

template<>
struct fmt::formatter<vk::PhysicalDevice> : formatter<std::string>
{
    template <typename FormatContext>
    auto format(const vk::PhysicalDevice& dev, FormatContext& ctx) const -> decltype(ctx.out()) {
        const vk::PhysicalDeviceProperties props = dev.getProperties();
        return fmt::format_to(ctx.out(), "{}, {}, apiVer: {}, devId: {}, driverVer: {}",
          static_cast<std::string_view>(props.deviceName),
          to_string(props.deviceType),
          props.apiVersion,
          props.deviceID,
          props.driverVersion);
    }
};

template<>
struct fmt::formatter<vk::Result> : formatter<std::string>
{
    template <typename FormatContext>
    auto format(const vk::Result& result, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", to_string(result));
    }
};

namespace volkano {

inline void vk_check_result(vk::Result result)
{
    VKE_ASSERT_MSG(result == vk::Result::eSuccess, "result: {}", result);
}

inline void vk_check_result(vk::Result result, std::span<vk::Result> invalid_results)
{
    VKE_ASSERT_MSG(!ranges::contains(invalid_results, result), "result: {}", result);
}

decltype(auto) vk_check_result(auto vk_ret)
{
    vk_check_result(vk_ret.result);
    return vk_ret.value;
}

} // namespace volkano

