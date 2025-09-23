
#include "vklite.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <GLFW/glfw3.h>

static void vklite_glfw_error_callback(int error, const char* description) {
  std::cerr << "GLFW error [" << error << "]: " << (description ? description : "<null>") << std::endl;
}

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
  std::cout << std::endl;

  // Query Vulkan loader for supported API version
  uint32_t apiVersion = 0;
  if (vkEnumerateInstanceVersion) {
    vkEnumerateInstanceVersion(&apiVersion);
  } else {
    apiVersion = VK_API_VERSION_1_0;
  }
  uint32_t major = VK_VERSION_MAJOR(apiVersion);
  uint32_t minor = VK_VERSION_MINOR(apiVersion);
  uint32_t patch = VK_VERSION_PATCH(apiVersion);
  std::cout << "Vulkan loader supports API version: " << major << "." << minor << "." << patch << std::endl;
  if (apiVersion < VK_API_VERSION_1_3) {
    std::cerr << "Vulkan 1.3 or higher is required!" << std::endl;
    return false;
  }

  // Query required extensions from GLFW
  // Install error callback before init so we catch failures
  glfwSetErrorCallback(vklite_glfw_error_callback);

  bool glfw_inited = glfwInit() != 0;
  if (!glfw_inited) {
    std::cerr << "Failed to initialize GLFW!" << std::endl;
    return false;
  }
  // We use Vulkan for rendering; tell GLFW not to create an OpenGL context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  // If validation is enabled, request debug utils extension
  std::vector<const char*> validationLayers;
  if (validation_enabled) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    validationLayers.push_back("VK_LAYER_KHRONOS_validation");
  }

#if defined(VKLITE_PLAT_MAC)
  // Add portability enumeration extension for macOS
  extensions.push_back("VK_KHR_portability_enumeration");
#endif

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
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();
  if (!validationLayers.empty()) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
#if defined(VKLITE_PLAT_MAC)
  createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance!" << std::endl;
    instance = VK_NULL_HANDLE;
    return false;
  }

  // Create debug messenger if validation enabled
  if (validation_enabled) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) {
      VkDebugUtilsMessengerCreateInfoEXT dbg{};
      dbg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      dbg.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                               VkDebugUtilsMessageTypeFlagsEXT types,
                               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                               void* pUserData) -> VkBool32 {
        Context* ctx = reinterpret_cast<Context*>(pUserData);
        if (ctx && ctx->validation_callback) {
          ctx->validation_callback(severity, types, std::string(pCallbackData->pMessage));
        }
        return VK_FALSE;
      };
      dbg.pUserData = this;
      func(instance, &dbg, nullptr, &debugMessenger);
    }
  }
  // --- Physical device selection ---
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    std::cerr << "No Vulkan physical devices found" << std::endl;
    return false;
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  // Pick the first device that has a graphics queue
  for (auto dev : devices) {
    uint32_t qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &qCount, nullptr);
    std::vector<VkQueueFamilyProperties> qprops(qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &qCount, qprops.data());
    for (uint32_t i = 0; i < qCount; ++i) {
      if (qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        physicalDevice = dev;
        graphicsQueueFamily = i;
        break;
      }
    }
    if (physicalDevice != VK_NULL_HANDLE) break;
  }
  if (physicalDevice == VK_NULL_HANDLE) {
    std::cerr << "Failed to find a suitable physical device with graphics queue" << std::endl;
    return false;
  }

  // --- Device creation ---
  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueCreate{};
  queueCreate.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreate.queueFamilyIndex = graphicsQueueFamily;
  queueCreate.queueCount = 1;
  queueCreate.pQueuePriorities = &queuePriority;

  // Required device extensions
  std::vector<const char*> deviceExtensions = { "VK_KHR_swapchain" };

  // Check for VK_KHR_dynamic_rendering support via extension availability
  uint32_t extCount = 0;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
  std::vector<VkExtensionProperties> extProps(extCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, extProps.data());
  bool dynamicRenderingAvailable = false;
  for (auto &e : extProps) {
    if (std::strcmp(e.extensionName, "VK_KHR_dynamic_rendering") == 0) {
      dynamicRenderingAvailable = true;
      deviceExtensions.push_back("VK_KHR_dynamic_rendering");
      break;
    }
  }

  VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
  dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
  dynamicRenderingFeature.pNext = nullptr;
  dynamicRenderingFeature.dynamicRendering = dynamicRenderingAvailable ? VK_TRUE : VK_FALSE;

  VkDeviceCreateInfo deviceCreate{};
  deviceCreate.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreate.pNext = dynamicRenderingAvailable ? &dynamicRenderingFeature : nullptr;
  deviceCreate.queueCreateInfoCount = 1;
  deviceCreate.pQueueCreateInfos = &queueCreate;
  deviceCreate.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  deviceCreate.ppEnabledExtensionNames = deviceExtensions.data();

  result = vkCreateDevice(physicalDevice, &deviceCreate, nullptr, &device);
  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create logical device" << std::endl;
    device = VK_NULL_HANDLE;
    return false;
  }

  vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);

  // Load device-level function pointers for dynamic rendering, if enabled
  if (dynamicRenderingAvailable) {
    vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR"));
    vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR"));
  }

  return true;
}


// Window-related methods are implemented in src/window.cpp

void Context::shutdown(){
  // Destroy all windows and their Vulkan resources first.
  // destroyWindow will call destroySwapchainForWindow and destroy the surface and GLFW window.
  while (!windows.empty()) {
    destroyWindow(windows.back().get());
  }

  // Terminate GLFW after windows are destroyed.
  glfwTerminate();

  // Ensure the device is idle and destroy it before destroying the instance.
  if (device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);
    device = VK_NULL_HANDLE;
    graphicsQueue = VK_NULL_HANDLE;
    graphicsQueueFamily = UINT32_MAX;
  }

  // Destroy debug messenger (uses instance) and then destroy the instance.
  if (instance != VK_NULL_HANDLE) {
    if (debugMessenger) {
      auto destroyFn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
      if (destroyFn) destroyFn(instance, debugMessenger, nullptr);
      debugMessenger = VK_NULL_HANDLE;
    }
    vkDestroyInstance(instance, nullptr);
    instance = VK_NULL_HANDLE;
  }

  std::cout << "vklite: shutdown" << std::endl;
}

} // namespace vklite
