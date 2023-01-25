/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <vector>

#include "engine/renderer/vk_include.h" // todo vulkan_handles.hpp
#include "engine/renderer/renderer_interface.h"
#include "engine/core/logging/logging.h"
#include "engine/core/filesystem/filesystem.h"

VKE_DECLARE_LOG_CATEGORY(vulkan);
VKE_DECLARE_LOG_CATEGORY(renderer);

namespace volkano {

struct vk_queue_family_indices {
    ptrdiff graphics_index = -1;
    ptrdiff present_index = -1;
    ptrdiff compute_index = -1;
    ptrdiff transfer_index = -1;
};

struct vk_surface_capabilities {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

class vk_renderer : public renderer_interface {
    engine* engine_;
    vk::Instance instance_ = nullptr;
    std::vector<vk::PhysicalDevice> physical_devices_;
    vk::PhysicalDevice current_physical_device_ = nullptr;
    vk_queue_family_indices queue_family_indices_;
    vk::Device logical_device_ = nullptr;

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

    static bool is_phys_device_suitable(vk::PhysicalDevice dev) noexcept;
};

} // namespace volkano