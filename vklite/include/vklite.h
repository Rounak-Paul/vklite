
#pragma once

// Vulkan header
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
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

  // Initialize creates the Vulkan instance and prepares internal state.
  // Returns true on success, false on failure.
  bool initialize(const std::string &appName = "vklite-app");

  // Create a new window. Returns pointer to Window struct.
  Window* createWindow(int width, int height, const std::string& title);

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

private:
  std::vector<std::unique_ptr<Window>> windows;
};

} // namespace vklite
