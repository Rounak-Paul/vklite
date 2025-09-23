
#include "vklite.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <GLFW/glfw3.h>

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
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW!" << std::endl;
    return false;
  }
  // We use Vulkan for rendering; tell GLFW not to create an OpenGL context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

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
#if defined(VKLITE_PLAT_MAC)
  createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance!" << std::endl;
    instance = VK_NULL_HANDLE;
    return false;
  }
  return true;
}


Window* Context::createWindow(int width, int height, const std::string& title) {
  GLFWwindow* win = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!win) {
    std::cerr << "Failed to create GLFW window: " << title << std::endl;
    return nullptr;
  }
  auto w = std::make_unique<Window>();
  w->handle = win;
  w->width = width;
  w->height = height;
  w->title = title;
  // Store the window in our managed list and return a raw pointer handle.
  windows.push_back(std::move(w));
  return windows.back().get();
}

void Context::destroyWindow(Window* window) {
  if (!window || !window->handle) return;
  GLFWwindow* gw = static_cast<GLFWwindow*>(window->handle);
  glfwDestroyWindow(gw);
  window->handle = nullptr;
  // Erase from our windows vector
  for (auto it = windows.begin(); it != windows.end(); ++it) {
    if (it->get() == window) {
      windows.erase(it);
      break;
    }
  }
}

bool Context::isWindowOpen(const Window* window) const {
  if (!window || !window->handle) return false;
  return !glfwWindowShouldClose(static_cast<GLFWwindow*>(window->handle));
}

void Context::pollEvents() {
  glfwPollEvents();
}

void Context::runMainLoop() {
  while (true) {
    // Poll platform/window events for all windows.
    pollEvents();

    // Collect windows that requested close and destroy them.
    std::vector<Window*> toDestroy;
    for (auto& up : windows) {
      Window* w = up.get();
      if (w && w->handle && glfwWindowShouldClose(static_cast<GLFWwindow*>(w->handle))) {
        toDestroy.push_back(w);
      }
    }

    for (Window* w : toDestroy) destroyWindow(w);

    if (windows.empty()) break;
  }
}

void Context::shutdown(){
  // Destroy all windows
  for (auto& w : windows) {
    if (w && w->handle) {
      glfwDestroyWindow(static_cast<GLFWwindow*>(w->handle));
      w->handle = nullptr;
    }
  }
  windows.clear();
  glfwTerminate();
  if (instance != VK_NULL_HANDLE) {
    vkDestroyInstance(instance, nullptr);
    instance = VK_NULL_HANDLE;
  }
  std::cout << "vklite: shutdown" << std::endl;
}

} // namespace vklite
