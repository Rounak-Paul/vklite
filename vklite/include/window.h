#pragma once

#include <string>
#include <memory>

// Forward declare GLFWwindow to keep header light; implementation will include GLFW.
struct GLFWwindow;

namespace vklite {

struct Window {
  void* handle = nullptr; // Will be GLFWwindow*
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  // dynamic rendering: no render pass/framebuffers are required
  // Per-window frame resources
  VkCommandPool commandPool = VK_NULL_HANDLE;
  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
  VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
  VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
  VkFence inFlightFence = VK_NULL_HANDLE;
  int width = 0;
  int height = 0;
  std::string title;
};

} // namespace vklite