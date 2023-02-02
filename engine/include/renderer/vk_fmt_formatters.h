/*
 * Copyright (C) 2023 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <fmt/format.h>
#include <fmt/color.h>

#include "core/int_types.h"
#include "renderer/vk_include.h"

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
        return fmt::format_to(ctx.out(), "{}, {}, apiVer: {}.{}.{}, devId: {}, driverVer: {}",
          static_cast<std::string_view>(props.deviceName),
          to_string(props.deviceType),
          VK_API_VERSION_MAJOR(props.apiVersion),
          VK_API_VERSION_MINOR(props.apiVersion),
          VK_API_VERSION_PATCH(props.apiVersion),
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

template<>
struct fmt::formatter<VkDebugUtilsLabelEXT> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const VkDebugUtilsLabelEXT& label, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", fmt::styled(label.pLabelName, fmt::fg(fmt::rgb{
          static_cast<volkano::u8>(label.color[0] * 255),
          static_cast<volkano::u8>(label.color[1] * 255),
          static_cast<volkano::u8>(label.color[2] * 255)
        })));
    }
};

template<>
struct fmt::formatter<VkDebugUtilsObjectNameInfoEXT> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const VkDebugUtilsObjectNameInfoEXT& info, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{} ({} at {:p})",
          info.pObjectName ? info.pObjectName : "unnamed",
          to_string(static_cast<vk::ObjectType>(info.objectType)),
          reinterpret_cast<const void*>(info.objectHandle));
    }
};
