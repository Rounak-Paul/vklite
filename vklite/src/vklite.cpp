#include "vklite.h"
#include <iostream>

namespace vklite {

bool Context::initialize(const std::string &appName){
  std::cout << "vklite: initialize for " << appName;
#if defined(VKLITE_PLAT_WINDOWS)
  std::cout << " [platform: windows]";
#elif defined(VKLITE_PLAT_MAC)
  std::cout << " [platform: macos]";
#elif defined(VKLITE_PLAT_LINUX)
  std::cout << " [platform: linux]";
#endif
  std::cout << "\n";
    // Vulkan instance creation
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.pEngineName = "vklite";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
      std::cerr << "Failed to create Vulkan instance!" << std::endl;
      instance = VK_NULL_HANDLE;
      return false;
    }
    return true;
}

void Context::shutdown(){
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
    std::cout << "vklite: shutdown" << std::endl;
}

} // namespace vklite
