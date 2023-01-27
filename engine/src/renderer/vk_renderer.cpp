/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <SDL2/SDL_vulkan.h>

#include "renderer/vk_renderer.h"
#include "version.h"
#include "core/algo/contains.h"
#include "core/container/static_vector.h"
#include "core/util/fmt_formatters.h"
#include "core/algo/index_of.h"

VKE_DEFINE_LOG_CATEGORY(vulkan, verbose);
VKE_DEFINE_LOG_CATEGORY(renderer, verbose);

namespace volkano {

namespace {

void validate_required_extensions(const auto required_extensions, const auto available_extensions)
{
    for (const char* extension : required_extensions) {
        const bool extension_is_available = algo::contains_if(
          available_extensions,
          [extension](const std::string_view available_extension) { return available_extension == extension; },
          &vk::ExtensionProperties::extensionName);

        VKE_ASSERT_MSG(extension_is_available, "required vulkan extension does not exist: {}", extension);
    }
}

} // namespace

void vk_renderer::initialize() noexcept
{
    create_vk_instance();
    create_surface();
    cache_physical_devices();
    create_graphics_pipeline();
    create_framebuffers();
    create_command_pool();
    create_sync_objects();
}

void vk_renderer::render() noexcept
{
    vk_check_result(logical_device_.waitForFences({in_flight_fence_}, /*waitAll=*/true, /*timeout=*/std::numeric_limits<u64>::max()));
    vk_check_result(logical_device_.resetFences({in_flight_fence_}));

    const u32 image_idx = vk_check_result(logical_device_.acquireNextImageKHR(
      swapchain_, /*timeout=*/std::numeric_limits<u64>::max(), image_available_semaphore_));
    command_buffer_.reset();
    record_command_buffer(image_idx);

    vk::PipelineStageFlags waitStages[]{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submit_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &image_available_semaphore_,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer_,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &render_finished_semaphore_
    };

    vk_check_result(graphics_queue_.submit({submit_info}, in_flight_fence_));

    vk::PresentInfoKHR present_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &render_finished_semaphore_,
      .swapchainCount = 1,
      .pSwapchains = &swapchain_,
      .pImageIndices = &image_idx
    };

    vk::Result present_result = present_queue_.presentKHR(present_info);
    if (present_result == vk::Result::eSuboptimalKHR || present_result == vk::Result::eErrorOutOfDateKHR) {
        on_window_resize();
    }
}

void vk_renderer::on_window_resize() noexcept
{
    destroy_surface_objects();

    create_swap_chain();
    create_framebuffers();
}

vk_renderer::~vk_renderer()
{
    if (logical_device_) {
        destroy_surface_objects();

        logical_device_.destroy(swapchain_);
        logical_device_.destroy(pipeline_layout_);
        logical_device_.destroy(pipeline_);
        logical_device_.destroy(render_pass_);
        logical_device_.destroy(command_pool_);
        logical_device_.destroy(image_available_semaphore_);
        logical_device_.destroy(render_finished_semaphore_);
        logical_device_.destroy(in_flight_fence_);
        logical_device_.destroy();
    }

    if (instance_) {
        instance_.destroy(surface_);
        instance_.destroy();
    }
}

void vk_renderer::create_vk_instance() noexcept
{
    const std::vector<vk::ExtensionProperties> available_instance_ext_properties = vk_check_result(vk::enumerateInstanceExtensionProperties());
    const std::vector<vk::LayerProperties> available_instance_layer_properties = vk_check_result(vk::enumerateInstanceLayerProperties());
    VKE_LOG(vulkan, verbose, "instance extension properties:\n\t{}", fmt::join(available_instance_ext_properties, "\n\t"));
    VKE_LOG(vulkan, verbose, "instance layer properties:\n\t{}", fmt::join(available_instance_layer_properties, "\n\t"));

    vk::ApplicationInfo appInfo{
      .pApplicationName = "volkano",
      .applicationVersion = VK_MAKE_VERSION(volkano::version_major, volkano::version_minor, volkano::version_patch),
      .pEngineName = "volkano",
      .engineVersion = VK_MAKE_VERSION(0, 0, 1),
      .apiVersion = VK_API_VERSION_1_3
    };

    static_vector<const char*, 8> instance_layers;

#if DEBUG
    {
        constexpr std::string_view validation_layer_name = "VK_LAYER_KHRONOS_validation";
        const bool validation_is_available = algo::contains(
          available_instance_layer_properties,
          validation_layer_name,
          &vk::LayerProperties::layerName);
        if (validation_is_available) {
            instance_layers.push_back(validation_layer_name.data());
        }
    };
#endif // DEBUG

    static_vector<const char*, 8> instance_extensions;
    {
        u32 n_sdl_extensions = 0;
        SDL_Vulkan_GetInstanceExtensions(engine_->get_window(), &n_sdl_extensions, nullptr);
        instance_extensions.resize(n_sdl_extensions);
        SDL_Vulkan_GetInstanceExtensions(engine_->get_window(), &n_sdl_extensions, instance_extensions.data());
    }

    validate_required_extensions(instance_extensions, available_instance_ext_properties);

    vk::InstanceCreateInfo create_info{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = instance_layers.size(),
      .ppEnabledLayerNames = instance_layers.data(),
      .enabledExtensionCount = instance_extensions.size(),
      .ppEnabledExtensionNames = instance_extensions.data()
    };

    instance_ = vk_check_result(vk::createInstance(create_info));
    VKE_LOG(vulkan, verbose, "vk instance created");
}

void vk_renderer::create_surface() noexcept
{
    VkSurfaceKHR surface;
    [[maybe_unused]] SDL_bool result = SDL_Vulkan_CreateSurface(engine_->get_window(), instance_, &surface);
    surface_ = surface;
    VKE_ASSERT_MSG(result == SDL_TRUE, "SDL could not create a Vulkan surface");
    VKE_LOG(vulkan, verbose, "vk surface created");
}

void vk_renderer::cache_physical_devices() noexcept
{
    physical_devices_ = vk_check_result(instance_.enumeratePhysicalDevices());
    // todo filter out unsuitable devices
    VKE_LOG(vulkan, verbose, "available physical devices:\n\t{}", fmt::join(physical_devices_, "\n\t"));

    // todo rate and sort devices instead of this based on type, max limits, and queue family availability, (possibly other stuff too)
    const auto found_it = std::ranges::find_if(physical_devices_, &vk_renderer::is_phys_device_suitable);
    VKE_ASSERT_MSG(found_it != physical_devices_.end(), "no suitable physical device was found to run vulkan");

    switch_physical_device_to(*found_it);
}

void vk_renderer::switch_physical_device_to(const vk::PhysicalDevice dev) noexcept
{
    current_physical_device_ = dev;
    VKE_LOG(renderer, info, "current physical dev: {}", current_physical_device_);

    populate_queue_family_indices();
    create_logical_device();
    cache_queues();
    create_swap_chain();
}

void vk_renderer::populate_queue_family_indices() noexcept
{
    std::vector<vk::QueueFamilyProperties> q_family_props = current_physical_device_.getQueueFamilyProperties();
    queue_family_indices_.graphics_index = algo::index_of_by_predicate(q_family_props,
      [](const vk::QueueFamilyProperties& props) {
          return (props.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
      });
    queue_family_indices_.present_index = algo::index_of_by_predicate(q_family_props,
      [&, idx = 0](const vk::QueueFamilyProperties&) mutable {
          return vk_check_result(current_physical_device_.getSurfaceSupportKHR(idx++, surface_));
      });
    queue_family_indices_.compute_index = algo::index_of_by_predicate(q_family_props,
      [](const vk::QueueFamilyProperties& props) {
          return (props.queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute;
      });
    queue_family_indices_.transfer_index = algo::index_of_by_predicate(q_family_props,
      [](const vk::QueueFamilyProperties& props) {
          return (props.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;
      });

    // todo make graphics optional in case engine is running without graphics
    VKE_LOG(renderer, verbose, "queue family indices seleced - graphics: {} present: {} compute: {} transfer: {}",
      queue_family_indices_.graphics_index,
      queue_family_indices_.present_index,
      queue_family_indices_.compute_index,
      queue_family_indices_.transfer_index);
}

void vk_renderer::create_logical_device() noexcept
{
    VKE_ASSERT(queue_family_indices_.graphics_index != -1);
    const float priority = 1.0f;
    vk::DeviceQueueCreateInfo device_queue_create_info{
      .queueFamilyIndex = static_cast<uint32_t>(queue_family_indices_.graphics_index),
      .queueCount = 1,
      .pQueuePriorities = &priority
    };

    std::vector<vk::ExtensionProperties> device_extension_properties = vk_check_result(current_physical_device_.enumerateDeviceExtensionProperties());

    static_vector<const char*, 16> device_extensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    validate_required_extensions(device_extensions, device_extension_properties);

    vk::PhysicalDeviceFeatures physical_device_features;
    vk::DeviceCreateInfo create_info{
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &device_queue_create_info,
      .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
      .ppEnabledExtensionNames = device_extensions.data(),
      .pEnabledFeatures = &physical_device_features,
    };

    logical_device_ = vk_check_result(current_physical_device_.createDevice(create_info));
    VKE_LOG(vulkan, verbose, "logical device created");
}

void vk_renderer::cache_queues() noexcept
{
    graphics_queue_ = logical_device_.getQueue(queue_family_indices_.graphics_index, 0);
    present_queue_ = logical_device_.getQueue(queue_family_indices_.present_index, 0);
    // compute_queue_ = logical_device_.getQueue(queue_family_indices_.compute_index, 0);
    // transfer_queue_ = logical_device_.getQueue(queue_family_indices_.transfer_index, 0);
}

void vk_renderer::create_swap_chain() noexcept
{
    surface_capabilities_.capabilities = vk_check_result(current_physical_device_.getSurfaceCapabilitiesKHR(surface_));
    surface_capabilities_.formats = vk_check_result(current_physical_device_.getSurfaceFormatsKHR(surface_));
    surface_capabilities_.present_modes = vk_check_result(current_physical_device_.getSurfacePresentModesKHR(surface_));

    const vk::SurfaceFormatKHR surface_fmt = [&]() {
        for (const auto& availableFormat : surface_capabilities_.formats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return surface_capabilities_.formats[0];
    }();
    VKE_LOG(vulkan, verbose, "swap chain surface format: {} color space: {}", surface_fmt.format, surface_fmt.colorSpace);

    const vk::PresentModeKHR present_mode = [&]() {
        for (const auto& availablePresentMode : surface_capabilities_.present_modes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }();
    VKE_LOG(vulkan, verbose, "swap chain present mode: {}", present_mode);

    surface_fmt_ = surface_fmt.format;
    extent_ = [&]() {
        if (surface_capabilities_.capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
            return surface_capabilities_.capabilities.currentExtent;
        } else {
            const vec2u win_extent = engine_->get_window_extent();
            vk::Extent2D actualExtent{win_extent.x, win_extent.y};

            actualExtent.width = std::clamp(actualExtent.width,
              surface_capabilities_.capabilities.minImageExtent.width,
              surface_capabilities_.capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height,
              surface_capabilities_.capabilities.minImageExtent.height,
              surface_capabilities_.capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }();

    vk::SwapchainCreateInfoKHR swapchain_create_info{
      .surface = surface_,
      .minImageCount = surface_capabilities_.capabilities.minImageCount + 1,
      .imageFormat = surface_fmt.format,
      .imageColorSpace = surface_fmt.colorSpace,
      .imageExtent = extent_,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = surface_capabilities_.capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = present_mode,
      .clipped = true,
      .oldSwapchain = swapchain_
    };

    swapchain_ = vk_check_result(logical_device_.createSwapchainKHR(swapchain_create_info));
    swapchain_images_ = vk_check_result(logical_device_.getSwapchainImagesKHR(swapchain_));

    swapchain_image_views_.reserve(swapchain_images_.size());
    for (const vk::Image img : swapchain_images_) {
        vk::ImageView view = vk_check_result(logical_device_.createImageView(
          vk::ImageViewCreateInfo{
            .image = img,
            .viewType = vk::ImageViewType::e2D,
            .format = surface_fmt_,
            .components = vk::ComponentMapping{
              .r = vk::ComponentSwizzle::eIdentity,
              .g = vk::ComponentSwizzle::eIdentity,
              .b = vk::ComponentSwizzle::eIdentity,
              .a = vk::ComponentSwizzle::eIdentity,
            },
            .subresourceRange = vk::ImageSubresourceRange{
              .aspectMask = vk::ImageAspectFlagBits::eColor,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1
            }
          }));
        swapchain_image_views_.emplace_back(view);
    }

    VKE_LOG(vulkan, verbose, "swapchain initialized");
}

bool vk_renderer::is_phys_device_suitable(const vk::PhysicalDevice dev) noexcept
{
    return dev.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
}

void vk_renderer::create_graphics_pipeline() noexcept
{
    const std::vector<u8> vert = fs::read_bytes_from_file("engine/shaders/triangle.vert.spr");
    const std::vector<u8> frag = fs::read_bytes_from_file("engine/shaders/triangle.frag.spr");

    vk::ShaderModule vert_module = create_shader_module(vert);
    vk::ShaderModule frag_module = create_shader_module(frag);

    static_vector<vk::PipelineShaderStageCreateInfo, 2> shader_stage_create_infos{
      vk::PipelineShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = vert_module,
        .pName = "main"
      },
      vk::PipelineShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = frag_module,
        .pName = "main"
      }
    };

    static_vector<vk::DynamicState, 2> dynamic_states{
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{
      .dynamicStateCount = dynamic_states.size(),
      .pDynamicStates = dynamic_states.data()
    };

    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{
      .vertexBindingDescriptionCount = 0,
      .pVertexBindingDescriptions = nullptr,
      .vertexAttributeDescriptionCount = 0,
      .pVertexAttributeDescriptions = nullptr
    };

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{
      .topology = vk::PrimitiveTopology::eTriangleList,
      .primitiveRestartEnable = false
    };

    vk::PipelineViewportStateCreateInfo viewport_state_create_info{
      .viewportCount = 1,
      .scissorCount = 1
    };

    vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{
      .depthClampEnable = false,
      .rasterizerDiscardEnable = false,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasClamp = false,
      .lineWidth = 1.f
    };

    vk::PipelineMultisampleStateCreateInfo multisample_state_create_info{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = false // msaa disabled
    };

    vk::PipelineColorBlendAttachmentState color_blend_attachment_state{
      .blendEnable = false,
      .srcColorBlendFactor = vk::BlendFactor::eOne,
      .dstColorBlendFactor = vk::BlendFactor::eZero,
      .colorBlendOp = vk::BlendOp::eAdd,
      .srcAlphaBlendFactor = vk::BlendFactor::eOne,
      .dstAlphaBlendFactor = vk::BlendFactor::eZero,
      .alphaBlendOp = vk::BlendOp::eAdd,
      .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
        | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info{
      .logicOpEnable = false,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &color_blend_attachment_state,
      .blendConstants = {{0.f, 0.f, 0.f, 0.f}}
    };

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_ = vk_check_result(logical_device_.createPipelineLayout(pipeline_layout_create_info));

    create_render_pass();

    vk::GraphicsPipelineCreateInfo pipeline_create_info{
      .stageCount = shader_stage_create_infos.size(),
      .pStages = shader_stage_create_infos.data(),
      .pVertexInputState = &vertex_input_state_create_info,
      .pInputAssemblyState = &input_assembly_state_create_info,
      .pViewportState = &viewport_state_create_info,
      .pRasterizationState = &rasterization_state_create_info,
      .pMultisampleState = &multisample_state_create_info,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &color_blend_state_create_info,
      .pDynamicState = &dynamic_state_create_info,
      .layout = pipeline_layout_,
      .renderPass = render_pass_,
      .subpass = 0
    };

    pipeline_ = vk_check_result(logical_device_.createGraphicsPipeline(nullptr, pipeline_create_info));

    logical_device_.destroy(vert_module);
    logical_device_.destroy(frag_module);

    VKE_LOG(vulkan, verbose, "graphics pipeline created");
}

void vk_renderer::create_render_pass() noexcept
{
    vk::AttachmentDescription color_attachment_desc{
      .format = surface_fmt_,
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };

    vk::AttachmentReference color_attachment_ref{
      .attachment = 0,
      .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    vk::SubpassDescription subpass_description{
      .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_ref,
    };

    vk::SubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
      .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
    };

    vk::RenderPassCreateInfo render_pass_create_info{
      .attachmentCount = 1,
      .pAttachments = &color_attachment_desc,
      .subpassCount = 1,
      .pSubpasses = &subpass_description,
      .dependencyCount = 1,
      .pDependencies = &dependency
    };

    render_pass_ = vk_check_result(logical_device_.createRenderPass(render_pass_create_info));
    VKE_LOG(vulkan, verbose, "render pass created");
};

void vk_renderer::create_framebuffers() noexcept
{
    swapchain_framebuffers_.reserve(swapchain_image_views_.size());
    for (vk::ImageView img_view : swapchain_image_views_) {
        vk::FramebufferCreateInfo framebuffer_create_info{
          .renderPass = render_pass_,
          .attachmentCount = 1,
          .pAttachments = &img_view,
          .width = extent_.width,
          .height = extent_.height,
          .layers = 1
        };

        vk::Framebuffer framebuffer = vk_check_result(logical_device_.createFramebuffer(framebuffer_create_info));
        swapchain_framebuffers_.emplace_back(framebuffer);
    }
    VKE_LOG(vulkan, verbose, "framebuffers created");
}

void vk_renderer::create_command_pool() noexcept
{
    vk::CommandPoolCreateInfo command_pool_create_info{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = static_cast<u32>(queue_family_indices_.graphics_index)
    };

    command_pool_ = vk_check_result(logical_device_.createCommandPool(command_pool_create_info));

    vk::CommandBufferAllocateInfo command_buffer_allocate_info{
      .commandPool = command_pool_,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1
    };

    command_buffer_ = vk_check_result(logical_device_.allocateCommandBuffers(command_buffer_allocate_info))[0];
    VKE_LOG(vulkan, verbose, "cmd buffer allocated");
}

void vk_renderer::create_sync_objects() noexcept
{
    image_available_semaphore_ = vk_check_result(logical_device_.createSemaphore({}));
    render_finished_semaphore_ = vk_check_result(logical_device_.createSemaphore({}));
    in_flight_fence_ = vk_check_result(logical_device_.createFence(vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled}));
}

void vk_renderer::destroy_surface_objects() noexcept
{
    vk_check_result(logical_device_.waitIdle());

    for (vk::ImageView view : swapchain_image_views_) {
        logical_device_.destroy(view);
    }

    for (vk::Framebuffer buffer : swapchain_framebuffers_) {
        logical_device_.destroy(buffer);
    }

    swapchain_images_.clear();
    swapchain_image_views_.clear();
    swapchain_framebuffers_.clear();
}

void vk_renderer::record_command_buffer(u32 img_index) noexcept
{
    vk::CommandBufferBeginInfo cmd_buffer_begin_info{};
    VKE_ASSERT(command_buffer_.begin(cmd_buffer_begin_info) == vk::Result::eSuccess);

    vk::ClearValue clear_color_value{
      .color = vk::ClearColorValue{{{0.f, 0.f, 0.f, 1.f}}}
    };
    vk::RenderPassBeginInfo render_pass_begin_info{
      .renderPass = render_pass_,
      .framebuffer = swapchain_framebuffers_[img_index],
      .renderArea = {
        .offset = {0, 0},
        .extent = extent_
      },
      .clearValueCount = 1,
      .pClearValues = &clear_color_value
    };

    command_buffer_.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
    command_buffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

    vk::Viewport viewport{
      .x = 0.f,
      .y = 0.f,
      .width = static_cast<float>(extent_.width),
      .height = static_cast<float>(extent_.height),
      .minDepth = 0.f,
      .maxDepth = 1.f
    };
    command_buffer_.setViewport(/*firstViewport=*/0, /*viewportCount=*/1, &viewport);

    vk::Rect2D scissor{
      .offset = {
        .x = 0,
        .y = 0
      },
      .extent = extent_
    };
    command_buffer_.setScissor(0, 1, &scissor);

    command_buffer_.draw(3, 1, 0, 0);
    command_buffer_.endRenderPass();

    VKE_ASSERT(command_buffer_.end() == vk::Result::eSuccess);
}

vk::ShaderModule vk_renderer::create_shader_module(std::span<const u8> spirv_binary) noexcept
{
    return vk_check_result(logical_device_.createShaderModule(vk::ShaderModuleCreateInfo{
      .codeSize = spirv_binary.size(),
      .pCode = reinterpret_cast<const u32*>(spirv_binary.data())
    }));
}

} // namespace volkano
