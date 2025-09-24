
#pragma once

// Vulkan header
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "window.h"

// Platform macros provided by the build system:
// - VKLITE_PLAT_WINDOWS (windows)
// - VKLITE_PLAT_LINUX   (linux / unix)
// - VKLITE_PLAT_MAC     (apple / macos)

namespace vklite {

class Context {
public:
  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  uint32_t graphicsQueueFamily = UINT32_MAX;
  // Validation / debug utils
  bool validation_enabled = true;
  // User-provided callback invoked when a validation message arrives.
  // Signature: (severity, messageType, message)
  std::function<void(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const std::string&)> validation_callback;
  VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

  // Initialize creates the Vulkan instance and prepares internal state.
  // Returns true on success, false on failure.
  bool initialize(const std::string &appName = "vklite-app");

  // Create a new window. Returns pointer to Window struct.
  Window* createWindow(int width, int height, const std::string& title);

  // Create/destroy per-window surface and swapchain helpers
  bool createSurfaceForWindow(Window* window);
  bool createSwapchainForWindow(Window* window);
  void destroySwapchainForWindow(Window* window);

  // Destroy a window.
  void destroyWindow(Window* window);

  // Returns true if the window is still open
  bool isWindowOpen(const Window* window) const;

  // Polls events for all windows
  void pollEvents();

  // Runs the main application loop. Returns when all windows are closed.
  void runMainLoop();

  // Get all windows.
  const std::vector<std::unique_ptr<Window>>& getWindows() const { return windows; }

  // Shutdown cleans up Vulkan objects and internal resources.
  void shutdown();

  // Device-level function pointers (optional features)
  PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = nullptr;
  PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = nullptr;

  // Simple pipeline abstraction for easy drawing from the sandbox.
  struct Pipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkShaderModule vert = VK_NULL_HANDLE;
    VkShaderModule frag = VK_NULL_HANDLE;
    // primitive vertex count used by draw call
    uint32_t vertexCount = 0;
  };

  // Destroy a pipeline
  void destroyPipeline(Pipeline* p);

  // Record draw commands for the provided pipeline into the given command buffer.
  // This is a convenience helper the sandbox can call inside the render callback.
  void recordPipelineDraw(Pipeline* p, Window* window, VkCommandBuffer cmdBuf);

  // Create a pipeline directly from GLSL source strings at runtime. This helper
  // will invoke an external glslangValidator binary to produce temporary SPIR-V
  // and then create the pipeline. Returns nullptr on failure.
  Pipeline* createPipelineFromGlsl(const std::string& vertGlsl, const std::string& fragGlsl, uint32_t vertexCount = 3, VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB);

  // When true perform GPU->CPU readback and print a small diagnostic per-frame.
  // Default false to avoid spamming output and slowing down runtime.
  bool debugReadback = false;

private:
  std::vector<std::unique_ptr<Window>> windows;
  // Render a single window (internal)
  void renderWindow(Window* window);
};

} // namespace vklite
