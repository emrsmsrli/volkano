/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <vector>

#include "vk_include.h"
#include "renderer_interface.h"
#include "core/logging/logging.h"
#include "core/filesystem/filesystem.h"

VKE_DECLARE_LOG_CATEGORY(vulkan);
VKE_DECLARE_LOG_CATEGORY(renderer);

namespace volkano {

struct vk_queue_family_indices {
    static constexpr u32 invalid_index = std::numeric_limits<u32>::max();

    u32 graphics_index = invalid_index;
    u32 present_index = invalid_index;
    u32 compute_index = invalid_index;
    u32 transfer_index = invalid_index;
};

struct vk_surface_capabilities {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

class vk_renderer : public renderer_interface {
    engine* engine_;
    u32 available_vk_version_ = VK_VERSION_1_3;
    vk::DynamicLoader dyn_loader_;
    vk::Instance instance_ = nullptr;
    std::vector<vk::PhysicalDevice> available_physical_devices_;
    vk::PhysicalDevice physical_device_ = nullptr;
    vk::Device device_ = nullptr;
    vk_queue_family_indices queue_family_indices_;

#if DEBUG
    vk::DebugUtilsMessengerEXT debug_messenger_ = nullptr;
#endif // DEBUG

    vk::SurfaceKHR surface_ = nullptr;
    vk_surface_capabilities surface_capabilities_;

    vk::Queue graphics_queue_ = nullptr;
    vk::Queue present_queue_ = nullptr;
    vk::Queue compute_queue_ = nullptr;
    vk::Queue transfer_queue_ = nullptr;

    vk::SwapchainKHR swapchain_ = nullptr;
    std::vector<vk::Image> swapchain_images_;
    std::vector<vk::ImageView> swapchain_image_views_;
    std::vector<vk::Framebuffer> swapchain_framebuffers_;

    vk::PipelineLayout pipeline_layout_ = nullptr;
    vk::RenderPass render_pass_ = nullptr;
    vk::Pipeline pipeline_ = nullptr;

    vk::CommandPool command_pool_ = nullptr;
    vk::CommandBuffer command_buffer_ = nullptr;

    vk::Semaphore image_available_semaphore_ = nullptr;
    vk::Semaphore render_finished_semaphore_ = nullptr;
    vk::Fence in_flight_fence_ = nullptr;

    vk::Extent2D extent_;
    vk::Format surface_fmt_ = vk::Format::eB8G8R8A8Srgb;

public:
    explicit vk_renderer(engine* engine) : engine_{engine} {}
    ~vk_renderer();

    void initialize() noexcept override;
    void render() noexcept override;

    void on_window_resize() noexcept override;

private:
    void create_vk_instance() noexcept;
    void create_surface() noexcept;
    void cache_physical_devices() noexcept;
    void switch_physical_device_to(vk::PhysicalDevice dev) noexcept;
    void populate_queue_family_indices() noexcept;
    void create_logical_device() noexcept;
    void cache_queues() noexcept;
    void create_swap_chain() noexcept;
    void create_graphics_pipeline() noexcept;
    void create_render_pass() noexcept;
    void create_framebuffers() noexcept;
    void create_command_pool() noexcept;
    void create_sync_objects() noexcept;

    void destroy_surface_objects() noexcept;

    void record_command_buffer(u32 img_index) noexcept;

    vk::ShaderModule create_shader_module(std::span<const u8> spirv_binary) noexcept;
};

} // namespace volkano
