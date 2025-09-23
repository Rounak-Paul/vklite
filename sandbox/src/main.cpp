#include "vklite.h"
#include <iostream>

int main() {
  vklite::Context ctx;
  if (!ctx.initialize("sandbox")) {
    std::cerr << "Failed to initialize vklite\n";
    return 1;
  }

  // Create two windows
  auto* win1 = ctx.createWindow(800, 600, "VkLite Window 1");
  auto* win2 = ctx.createWindow(640, 480, "VkLite Window 2");
  if (!win1 || !win2) {
    std::cerr << "Failed to create one or more windows\n";
    ctx.shutdown();
    return 1;
  }

  std::cout << " ctx.windows=" << ctx.getWindows().size() << "\n";
  std::cout << "sandbox running... (close all windows to exit)\n";
  ctx.runMainLoop();

  ctx.shutdown();
  return 0;
}
