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
  // Format of swapchain images (set when swapchain created)
  VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
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
  // Optional pipeline that will be drawn each frame into this window.
  // Managed by the application (sandbox) via Context::createPipelineFromSpv / destroyPipeline.
  struct VkPipelinePlaceholder; // forward only symbol for header cleanliness
  void* pipeline = nullptr; // will actually be Context::Pipeline*
};

} // namespace vklite