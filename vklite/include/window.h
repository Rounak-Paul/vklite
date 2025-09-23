#pragma once

#include <string>
#include <memory>

// Forward declare GLFWwindow to keep header light; implementation will include GLFW.
struct GLFWwindow;

namespace vklite {

struct Window {
  void* handle = nullptr; // Will be GLFWwindow*
  // Future: VkSurfaceKHR surface; VkSwapchainKHR swapchain; etc.
  int width = 0;
  int height = 0;
  std::string title;
};

} // namespace vklite