/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "renderer/vk_renderer.h"

#include <span>

#include <SDL2/SDL_vulkan.h>
#include <range/v3/action/sort.hpp>
#include <range/v3/algorithm/contains.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>

#include "version.h"
#include "core/algo/contains_if.h"
#include "core/container/static_vector.h"
#include "core/util/fmt_formatters.h"
#include "renderer/vk_fmt_formatters.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

VKE_DEFINE_LOG_CATEGORY(vulkan_general, verbose);
VKE_DEFINE_LOG_CATEGORY(vulkan_validation, verbose);
VKE_DEFINE_LOG_CATEGORY(vulkan_performance, verbose);
VKE_DEFINE_LOG_CATEGORY(vulkan_dev_addr_binding, verbose);
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

VkBool32 VKAPI_PTR debug_utils_messenger_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* /*pUserData*/)
{
    const log_verbosity verbosity = [&]() {
        switch (static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) {
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError: return log_verbosity::error;
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: return log_verbosity::warning;
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo: return log_verbosity::info;
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose: return log_verbosity::verbose;
            default: unreachable();
        }
    }();

    const auto make_labels_str = [](std::span<const VkDebugUtilsLabelEXT> labels, const std::string_view type) {
        if (labels.empty()) {
            return std::string{};
        }

        return fmt::format("\n{}:\n\t{}", type, fmt::join(labels, "\n\t"));
    };

    const std::string obj_names = [pCallbackData]() {
        if (pCallbackData->objectCount == 0) {
            return std::string{};
        }

        return fmt::format("\nobjects:\n\t{}",
          fmt::join(std::span{pCallbackData->pObjects, pCallbackData->objectCount}, "\n\t"));
    }();

#define VKE_DEBUG_UTILS_LOG(category) VKE_LOG_DYN(category, verbosity, "{}({}): {}{}{}",                        \
      pCallbackData->pMessageIdName,                                                                            \
      pCallbackData->messageIdNumber,                                                                           \
      pCallbackData->pMessage,                                                                                  \
      obj_names,                                                                                                \
      make_labels_str(std::span{pCallbackData->pQueueLabels, pCallbackData->queueLabelCount}, "queues"),        \
      make_labels_str(std::span{pCallbackData->pCmdBufLabels, pCallbackData->cmdBufLabelCount}, "cmd buffers"))

    switch (static_cast<vk::DebugUtilsMessageTypeFlagBitsEXT>(messageType)) {
        case vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
            VKE_DEBUG_UTILS_LOG(renderer);
            break;
        case vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
            VKE_DEBUG_UTILS_LOG(vulkan_validation);
            break;
        case vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
            VKE_DEBUG_UTILS_LOG(vulkan_performance);
            break;
        case vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding:
            VKE_DEBUG_UTILS_LOG(vulkan_dev_addr_binding);
            break;
        default: unreachable();
    }

#undef VKE_DEBUG_UTILS_LOG

    // vulkan expects false from this callback
    return VK_FALSE;
}

u32 rate_physical_device(const vk::PhysicalDevice dev) noexcept
{
    // todo rate based on type, max limits, and queue family availability, (possibly other stuff too)
    u32 rating = 0;
    const vk::PhysicalDeviceProperties properties = dev.getProperties();
    const vk::PhysicalDeviceFeatures features = dev.getFeatures();

    switch (properties.deviceType) {
        case vk::PhysicalDeviceType::eDiscreteGpu:
            rating += 3000;
            break;
        case vk::PhysicalDeviceType::eIntegratedGpu:
            rating += 1000;
            break;
        case vk::PhysicalDeviceType::eVirtualGpu:
            rating += 50;
            break;
        case vk::PhysicalDeviceType::eOther:
        case vk::PhysicalDeviceType::eCpu:
            break;
    }

    return rating;
}

} // namespace

void vk_renderer::initialize() noexcept
{
    VKE_ASSERT(dyn_loader_.success());

    create_vk_instance();
    create_surface();
    cache_physical_devices();
    create_graphics_pipeline();

    const vma::VulkanFunctions vk_funcs = vma::functionsFromDispatcher(VULKAN_HPP_DEFAULT_DISPATCHER);
    allocator_ = vk_check_result(vma::createAllocator(vma::AllocatorCreateInfo{
      .physicalDevice = physical_device_,
      .device = device_,
      .pVulkanFunctions = &vk_funcs,
      .instance = instance_,
      .vulkanApiVersion = available_vk_version_
    }));

    create_framebuffers();
    create_vertex_buffer();
    create_command_pool();
    create_sync_objects();
}

void vk_renderer::render() noexcept
{
    vk_check_result(device_.waitForFences({in_flight_fence_}, /*waitAll=*/true, /*timeout=*/std::numeric_limits<u64>::max()));
    vk_check_result(device_.resetFences({in_flight_fence_}));

    const u32 image_idx = vk_check_result(device_.acquireNextImageKHR(
      swapchain_, /*timeout=*/std::numeric_limits<u64>::max(), image_available_semaphore_));
    command_buffer_.reset();
    record_command_buffer(image_idx);

    const vk::PipelineStageFlags wait_stage{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const vk::SubmitInfo submit_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &image_available_semaphore_,
      .pWaitDstStageMask = &wait_stage,
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer_,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &render_finished_semaphore_
    };

    vk_check_result(graphics_queue_.submit({submit_info}, in_flight_fence_));

    const vk::PresentInfoKHR present_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &render_finished_semaphore_,
      .swapchainCount = 1,
      .pSwapchains = &swapchain_,
      .pImageIndices = &image_idx
    };

    const vk::Result present_result = present_queue_.presentKHR(present_info);
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
    if (device_) {
        destroy_surface_objects();

        allocator_.destroyBuffer(mesh_buffer_, mesh_buffer_allocation_);
        allocator_.destroy();

        device_.destroy(swapchain_);
        device_.destroy(pipeline_layout_);
        device_.destroy(pipeline_);
        device_.destroy(render_pass_);
        device_.destroy(command_pool_);
        device_.destroy(image_available_semaphore_);
        device_.destroy(render_finished_semaphore_);
        device_.destroy(in_flight_fence_);
        device_.destroy();
    }

    if (instance_) {
#if DEBUG
        instance_.destroy(debug_messenger_);
#endif // DEBUG
        instance_.destroy(surface_);
        instance_.destroy();
    }
}

void vk_renderer::create_vk_instance() noexcept
{
    VULKAN_HPP_DEFAULT_DISPATCHER.init(dyn_loader_.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

    const std::vector<vk::ExtensionProperties> available_instance_ext_properties = vk_check_result(vk::enumerateInstanceExtensionProperties());
    const std::vector<vk::LayerProperties> available_instance_layer_properties = vk_check_result(vk::enumerateInstanceLayerProperties());
    VKE_LOG(renderer, verbose, "instance extension properties:\n\t{}", fmt::join(available_instance_ext_properties, "\n\t"));
    VKE_LOG(renderer, verbose, "instance layer properties:\n\t{}", fmt::join(available_instance_layer_properties, "\n\t"));

    available_vk_version_ = vk_check_result(vk::enumerateInstanceVersion());
    VKE_LOG(renderer, verbose, "vk api ver {}.{}.{}",
      VK_API_VERSION_MAJOR(available_vk_version_),
      VK_API_VERSION_MINOR(available_vk_version_),
      VK_API_VERSION_PATCH(available_vk_version_));

    const vk::ApplicationInfo appInfo{
      .pApplicationName = "volkano",
      .applicationVersion = VK_MAKE_VERSION(volkano::version_major, volkano::version_minor, volkano::version_patch),
      .pEngineName = "volkano",
      .engineVersion = VK_MAKE_VERSION(0, 0, 1),
      .apiVersion = available_vk_version_
    };

    static_vector<const char*, 8> instance_layers;

#if DEBUG
    {
        constexpr std::string_view validation_layer_name = "VK_LAYER_KHRONOS_validation";
        const bool validation_is_available = ranges::contains(
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
#if DEBUG
        instance_extensions.push_back("VK_EXT_debug_utils");
#endif // DEBUG
    }

    validate_required_extensions(instance_extensions, available_instance_ext_properties);

    const vk::InstanceCreateInfo create_info{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = instance_layers.size(),
      .ppEnabledLayerNames = instance_layers.data(),
      .enabledExtensionCount = instance_extensions.size(),
      .ppEnabledExtensionNames = instance_extensions.data()
    };

    instance_ = vk_check_result(vk::createInstance(create_info));
    VKE_LOG(renderer, verbose, "vk instance created");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);

#if DEBUG
    vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{
      .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
      .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding,
      .pfnUserCallback = debug_utils_messenger_callback
    };

    debug_messenger_ = vk_check_result(instance_.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info));
#endif // DEBUG
}

void vk_renderer::create_surface() noexcept
{
    VkSurfaceKHR surface;
    [[maybe_unused]] SDL_bool result = SDL_Vulkan_CreateSurface(engine_->get_window(), instance_, &surface);
    surface_ = surface;
    VKE_ASSERT_MSG(result == SDL_TRUE, "SDL could not create a Vulkan surface");
    VKE_LOG(renderer, verbose, "vk surface created");
}

void vk_renderer::cache_physical_devices() noexcept
{
    available_physical_devices_ = vk_check_result(instance_.enumeratePhysicalDevices());
    VKE_LOG(renderer, verbose, "all physical devices:\n\t{}", fmt::join(available_physical_devices_, "\n\t"));

    available_physical_devices_ = available_physical_devices_
      | ranges::views::filter([](const vk::PhysicalDevice dev) { return rate_physical_device(dev) != 0; })
      | ranges::to<std::vector>()
      | ranges::actions::sort([](const vk::PhysicalDevice r, const vk::PhysicalDevice l) {
          return rate_physical_device(r) > rate_physical_device(l);
      });

    VKE_ASSERT_MSG(!available_physical_devices_.empty(), "no suitable physical device was found to run vulkan");
    VKE_LOG(renderer, verbose, "available physical devices:\n\t{}", fmt::join(available_physical_devices_, "\n\t"));

    switch_physical_device_to(available_physical_devices_.front());
}

void vk_renderer::switch_physical_device_to(const vk::PhysicalDevice dev) noexcept
{
    physical_device_ = dev;
    available_vk_version_ = dev.getProperties().apiVersion;
    VKE_LOG(renderer, info, "current physical dev: {} apiVer: {}.{}.{}", physical_device_,
      VK_API_VERSION_MAJOR(available_vk_version_),
      VK_API_VERSION_MINOR(available_vk_version_),
      VK_API_VERSION_PATCH(available_vk_version_));

    populate_queue_family_indices();
    create_logical_device();
    cache_queues();
    create_swap_chain();
}

void vk_renderer::populate_queue_family_indices() noexcept
{
    // todo improve this logic
    const std::vector<vk::QueueFamilyProperties> q_family_props = physical_device_.getQueueFamilyProperties();
    for (u32 idx = 0; const vk::QueueFamilyProperties& props : q_family_props) {
        if (queue_family_indices_.graphics_index == vk_queue_family_indices::invalid_index &&
          props.queueFlags & vk::QueueFlagBits::eGraphics) {
            queue_family_indices_.graphics_index = idx;

            // prefer the same queue for presentation as graphics
            if (queue_family_indices_.present_index == vk_queue_family_indices::invalid_index &&
              vk_check_result(physical_device_.getSurfaceSupportKHR(idx, surface_))) {
                queue_family_indices_.present_index = idx;
            }
        }

        // prefer separate transfer queue
        constexpr auto standalone_transfer_q_flags = ~(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute) | vk::QueueFlagBits::eTransfer;
        if (queue_family_indices_.transfer_index == vk_queue_family_indices::invalid_index &&
          props.queueFlags & standalone_transfer_q_flags) {
            queue_family_indices_.transfer_index = idx;
        }

        // prefer separate compute queue
        constexpr auto standalone_compute_q_flags = ~(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer) | vk::QueueFlagBits::eCompute;
        if (queue_family_indices_.compute_index == vk_queue_family_indices::invalid_index &&
          props.queueFlags & standalone_compute_q_flags) {
            queue_family_indices_.compute_index = idx;
        }

        ++idx;
    }

    const auto default_family_index = [&q_family_props](u32& out_idx, auto lambda) {
        if (out_idx == vk_queue_family_indices::invalid_index) {
            const auto it = std::ranges::find_if(q_family_props, lambda);
            if (it != q_family_props.end()) {
                out_idx = static_cast<u32>(std::ranges::distance(q_family_props.begin(), it));
            }
        }
    };

    default_family_index(queue_family_indices_.present_index,
      [&, idx = 0](const vk::QueueFamilyProperties&) mutable {
          return vk_check_result(physical_device_.getSurfaceSupportKHR(idx++, surface_));
      });
    default_family_index(queue_family_indices_.transfer_index,
      [](const vk::QueueFamilyProperties& props) {
          return (props.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;
      });
    default_family_index(queue_family_indices_.compute_index,
      [](const vk::QueueFamilyProperties& props) {
          return (props.queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute;
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
    VKE_ASSERT(queue_family_indices_.graphics_index != vk_queue_family_indices::invalid_index);
    const float priority = 1.0f;
    const vk::DeviceQueueCreateInfo device_queue_create_info{
      .queueFamilyIndex = queue_family_indices_.graphics_index,
      .queueCount = 1,
      .pQueuePriorities = &priority
    };

    const std::vector<vk::ExtensionProperties> device_extension_properties = vk_check_result(physical_device_.enumerateDeviceExtensionProperties());
    VKE_LOG(renderer, verbose, "device extension properties:\n\t{}", fmt::join(device_extension_properties, "\n\t"));

    const static_vector<const char*, 4> device_extensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    validate_required_extensions(device_extensions, device_extension_properties);

    const vk::PhysicalDeviceFeatures physical_device_features;
    const vk::DeviceCreateInfo create_info{
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &device_queue_create_info,
      .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
      .ppEnabledExtensionNames = device_extensions.data(),
      .pEnabledFeatures = &physical_device_features,
    };

    device_ = vk_check_result(physical_device_.createDevice(create_info));
    VKE_LOG(renderer, verbose, "logical device created");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device_);
}

void vk_renderer::cache_queues() noexcept
{
    graphics_queue_ = device_.getQueue(queue_family_indices_.graphics_index, /*queueIndex=*/0);
    present_queue_ = device_.getQueue(queue_family_indices_.present_index, /*queueIndex=*/0);
    // compute_queue_ = device_.getQueue(queue_family_indices_.compute_index, /*queueIndex=*/0);
    // transfer_queue_ = device_.getQueue(queue_family_indices_.transfer_index, /*queueIndex=*/0);
}

void vk_renderer::create_swap_chain() noexcept
{
    surface_capabilities_.capabilities = vk_check_result(physical_device_.getSurfaceCapabilitiesKHR(surface_));
    surface_capabilities_.formats = vk_check_result(physical_device_.getSurfaceFormatsKHR(surface_));
    surface_capabilities_.present_modes = vk_check_result(physical_device_.getSurfacePresentModesKHR(surface_));

    const vk::SurfaceFormatKHR surface_fmt = [&]() {
        for (const auto& availableFormat : surface_capabilities_.formats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return surface_capabilities_.formats.front();
    }();
    VKE_LOG(renderer, verbose, "swap chain surface format: {} color space: {}", surface_fmt.format, surface_fmt.colorSpace);

    const vk::PresentModeKHR present_mode = [&]() {
        for (const auto& availablePresentMode : surface_capabilities_.present_modes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }();
    VKE_LOG(renderer, verbose, "swap chain present mode: {}", present_mode);

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

    const vk::SwapchainCreateInfoKHR swapchain_create_info{
      .surface = surface_,
      .minImageCount = surface_capabilities_.capabilities.minImageCount + 1,
      .imageFormat = surface_fmt_,
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

    swapchain_ = vk_check_result(device_.createSwapchainKHR(swapchain_create_info));
    swapchain_images_ = vk_check_result(device_.getSwapchainImagesKHR(swapchain_));

    if (swapchain_create_info.oldSwapchain) {
        device_.destroy(swapchain_create_info.oldSwapchain);
    }

    swapchain_image_views_.reserve(swapchain_images_.size());
    for (const vk::Image img : swapchain_images_) {
        const vk::ImageView view = vk_check_result(device_.createImageView(
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

    VKE_LOG(renderer, verbose, "swapchain initialized");
}

void vk_renderer::create_graphics_pipeline() noexcept
{
    const std::vector<u8> vert = fs::read_bytes_from_file("engine/shaders/triangle.vert.spr");
    const std::vector<u8> frag = fs::read_bytes_from_file("engine/shaders/triangle.frag.spr");

    const vk::ShaderModule vert_module = create_shader_module(vert);
    const vk::ShaderModule frag_module = create_shader_module(frag);

    const static_vector<vk::PipelineShaderStageCreateInfo, 2> shader_stage_create_infos{
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

    const static_vector<vk::DynamicState, 2> dynamic_states{
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor
    };

    const vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{
      .dynamicStateCount = dynamic_states.size(),
      .pDynamicStates = dynamic_states.data()
    };

    const vk::VertexInputBindingDescription binding_description{
      .binding = 0,
      .stride = sizeof(vertex),
      .inputRate = vk::VertexInputRate::eVertex
    };

    const static_vector<vk::VertexInputAttributeDescription, 2> attr_descriptions{
      vk::VertexInputAttributeDescription{
        .location = 0,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = static_cast<u32>(offsetof(vertex, position))
      },
      vk::VertexInputAttributeDescription{
        .location = 1,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = static_cast<u32>(offsetof(vertex, color))
      }
    };

    const vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &binding_description,
      .vertexAttributeDescriptionCount = attr_descriptions.size(),
      .pVertexAttributeDescriptions = attr_descriptions.data()
    };

    const vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{
      .topology = vk::PrimitiveTopology::eTriangleList,
      .primitiveRestartEnable = false
    };

    const vk::PipelineViewportStateCreateInfo viewport_state_create_info{
      .viewportCount = 1,
      .scissorCount = 1
    };

    const vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{
      .depthClampEnable = false,
      .rasterizerDiscardEnable = false,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasClamp = false,
      .lineWidth = 1.f
    };

    const vk::PipelineMultisampleStateCreateInfo multisample_state_create_info{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = false // msaa disabled
    };

    const vk::PipelineColorBlendAttachmentState color_blend_attachment_state{
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

    const vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info{
      .logicOpEnable = false,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &color_blend_attachment_state,
      .blendConstants = {{0.f, 0.f, 0.f, 0.f}}
    };

    const vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_ = vk_check_result(device_.createPipelineLayout(pipeline_layout_create_info));

    create_render_pass();

    const vk::GraphicsPipelineCreateInfo pipeline_create_info{
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

    pipeline_ = vk_check_result(device_.createGraphicsPipeline(nullptr, pipeline_create_info));

    device_.destroy(vert_module);
    device_.destroy(frag_module);

    VKE_LOG(renderer, verbose, "graphics pipeline created");
}

void vk_renderer::create_render_pass() noexcept
{
    const vk::AttachmentDescription color_attachment_desc{
      .format = surface_fmt_,
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };

    const vk::AttachmentReference color_attachment_ref{
      .attachment = 0,
      .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    const vk::SubpassDescription subpass_description{
      .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_ref,
    };

    const vk::SubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
      .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
    };

    const vk::RenderPassCreateInfo render_pass_create_info{
      .attachmentCount = 1,
      .pAttachments = &color_attachment_desc,
      .subpassCount = 1,
      .pSubpasses = &subpass_description,
      .dependencyCount = 1,
      .pDependencies = &dependency
    };

    render_pass_ = vk_check_result(device_.createRenderPass(render_pass_create_info));
    VKE_LOG(renderer, verbose, "render pass created");
}

void vk_renderer::create_framebuffers() noexcept
{
    swapchain_framebuffers_.reserve(swapchain_image_views_.size());
    for (const vk::ImageView img_view : swapchain_image_views_) {
        const vk::FramebufferCreateInfo framebuffer_create_info{
          .renderPass = render_pass_,
          .attachmentCount = 1,
          .pAttachments = &img_view,
          .width = extent_.width,
          .height = extent_.height,
          .layers = 1
        };

        const vk::Framebuffer framebuffer = vk_check_result(device_.createFramebuffer(framebuffer_create_info));
        swapchain_framebuffers_.emplace_back(framebuffer);
    }
    VKE_LOG(renderer, verbose, "framebuffers created");
}

void vk_renderer::create_vertex_buffer() noexcept
{
    const mesh_buffer<vertex>& vert_buf = triangle_mesh_.get_vertex_buffer();
    const vk::BufferCreateInfo buffer_create_info{
      .size = vk::DeviceSize{vert_buf.size_in_bytes()},
      .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      .sharingMode = vk::SharingMode::eExclusive,
    };

    const vma::AllocationCreateInfo alloc_create_info{
      .flags = vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessRandom,
      .usage = vma::MemoryUsage::eAuto,
      .preferredFlags = vk::MemoryPropertyFlagBits::eHostCoherent
    };

    vma::AllocationInfo alloc_info;
    std::tie(mesh_buffer_, mesh_buffer_allocation_) =
      vk_check_result(allocator_.createBuffer(buffer_create_info, alloc_create_info, alloc_info));

    std::memcpy(alloc_info.pMappedData, vert_buf.data(), vert_buf.size_in_bytes());
    allocator_.flushAllocation(mesh_buffer_allocation_, alloc_info.offset, alloc_info.size);
}

void vk_renderer::create_command_pool() noexcept
{
    const vk::CommandPoolCreateInfo command_pool_create_info{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = static_cast<u32>(queue_family_indices_.graphics_index)
    };

    command_pool_ = vk_check_result(device_.createCommandPool(command_pool_create_info));

    const vk::CommandBufferAllocateInfo command_buffer_allocate_info{
      .commandPool = command_pool_,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1
    };

    command_buffer_ = vk_check_result(device_.allocateCommandBuffers(command_buffer_allocate_info)).front();
    VKE_LOG(renderer, verbose, "cmd buffer allocated");
}

void vk_renderer::create_sync_objects() noexcept
{
    image_available_semaphore_ = vk_check_result(device_.createSemaphore({}));
    render_finished_semaphore_ = vk_check_result(device_.createSemaphore({}));
    in_flight_fence_ = vk_check_result(device_.createFence(vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled}));
}

void vk_renderer::destroy_surface_objects() noexcept
{
    vk_check_result(device_.waitIdle());

    for (vk::ImageView view : swapchain_image_views_) {
        device_.destroy(view);
    }

    for (vk::Framebuffer buffer : swapchain_framebuffers_) {
        device_.destroy(buffer);
    }

    swapchain_images_.clear();
    swapchain_image_views_.clear();
    swapchain_framebuffers_.clear();
}

void vk_renderer::record_command_buffer(u32 img_index) noexcept
{
    vk::CommandBufferBeginInfo cmd_buffer_begin_info{};
    VKE_ASSERT(command_buffer_.begin(cmd_buffer_begin_info) == vk::Result::eSuccess);

    const vk::ClearValue clear_color_value{
      .color = vk::ClearColorValue{{{0.f, 0.f, 0.f, 1.f}}}
    };
    const vk::RenderPassBeginInfo render_pass_begin_info{
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

    const vk::Viewport viewport{
      .x = 0.f,
      .y = 0.f,
      .width = static_cast<float>(extent_.width),
      .height = static_cast<float>(extent_.height),
      .minDepth = 0.f,
      .maxDepth = 1.f
    };
    command_buffer_.setViewport(/*firstViewport=*/0, /*viewportCount=*/1, &viewport);

    const vk::Rect2D scissor{
      .offset = {
        .x = 0,
        .y = 0
      },
      .extent = extent_
    };
    command_buffer_.setScissor(0, 1, &scissor);

    std::array buffers{mesh_buffer_};
    std::array offsets{vk::DeviceSize{0}};
    command_buffer_.bindVertexBuffers(0, buffers, offsets);
    command_buffer_.draw(triangle_mesh_.get_vertex_buffer().size(), 1, 0, 0);
    command_buffer_.endRenderPass();

    VKE_ASSERT(command_buffer_.end() == vk::Result::eSuccess);
}

vk::ShaderModule vk_renderer::create_shader_module(std::span<const u8> spirv_binary) noexcept
{
    return vk_check_result(device_.createShaderModule(vk::ShaderModuleCreateInfo{
      .codeSize = spirv_binary.size(),
      .pCode = reinterpret_cast<const u32*>(spirv_binary.data())
    }));
}

} // namespace volkano
