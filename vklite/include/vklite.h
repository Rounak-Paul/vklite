#pragma once

// Vulkan header
#include <vulkan/vulkan.h>
#include <string>

// Platform macros provided by the build system:
// - VKLITE_PLAT_WINDOWS (windows)
// - VKLITE_PLAT_LINUX   (linux / unix)
// - VKLITE_PLAT_MAC     (apple / macos)

namespace vklite {


struct Context {
  VkInstance instance = VK_NULL_HANDLE;

  // Initialize creates the Vulkan instance and prepares internal state.
  // Returns true on success, false on failure.
  bool initialize(const std::string &appName = "vklite-app");

  // Shutdown cleans up Vulkan objects and internal resources.
  void shutdown();
};

} // namespace vklite
