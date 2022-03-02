#include <iostream>
#include <string_view>

#include <GLFW/glfw3.h>
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include <engine/logging/logging.h>

VK_DEFINE_LOG_CATEGORY_STATIC(test, verbose);

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    VK_LOG(test, critical, "test {}", 39);
    VK_LOG(test, error, "test {}", 40);
    VK_LOG(test, warning, "test {}", 41);
    VK_LOG(test, info, "test {}", 42);
    VK_LOG(test, debug, "test {}", 43);
    VK_LOG(test, verbose, "test {}", 44);

    glfwInit();
    glfwInitHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(500, 500, "Volkano", nullptr, nullptr);

    vk::ApplicationInfo app_info{"Volkano", VK_MAKE_VERSION(0, 0, 1), "Volkano Engine", VK_MAKE_VERSION(0, 0, 1), VK_API_VERSION_1_2};

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    vk::InstanceCreateInfo instance_create_info{vk::InstanceCreateFlags{}, &app_info, 0, nullptr, glfwExtensionCount, glfwExtensions};

    const auto [ic_result, instance] = vk::createInstanceUnique(instance_create_info);
    if(ic_result != vk::Result::eSuccess) {
        return -1;
    }

    const auto [eiep_result, extension_properties] = vk::enumerateInstanceExtensionProperties(nullptr);
    if(eiep_result != vk::Result::eSuccess) {
        return -1;
    }

    for (const auto &item : extension_properties) {
        std::cout << item.extensionName << '\n';
    }

    const auto rr = instance->enumeratePhysicalDeviceGroups();
    if(rr.result != vk::Result::eSuccess) {
        return -1;
    }

    std::cout << "-----------\n";
    for (const auto& group_props : rr.value) {
        for (const auto& device : group_props.physicalDevices) {
            if(device) {
                std::cout << device.getProperties().deviceName << '\n';
            }
        }
    }

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
