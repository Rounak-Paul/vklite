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

  // Create a pipeline from GLSL source strings at runtime (triangle only)
  // Full-screen triangle vertex shader (no vertex buffers)
  const std::string triVert = R"GLSL(#version 450
void main() {
  // Positions that form a full-screen triangle that covers the viewport
  vec2 positions[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
})GLSL";
  const std::string triFrag = R"GLSL(#version 450
layout(location = 0) out vec4 outColor;
void main() { outColor = vec4(1.0, 0.0, 0.0, 1.0); }
)GLSL";
  // Create pipeline using the swapchain's color format
  VkFormat colorFmt = (win1 && win1->swapchainFormat != VK_FORMAT_UNDEFINED) ? win1->swapchainFormat : VK_FORMAT_B8G8R8A8_SRGB;
  auto* triPipeline = ctx.createPipelineFromGlsl(triVert, triFrag, 3, colorFmt);
  if (triPipeline) {
    win1->pipeline = triPipeline;
    std::cout << "Triangle pipeline created successfully.\n";
  } else {
    std::cout << "Failed to create triangle pipeline. Check shader compiler output above.\n";
  }

  std::cout << " ctx.windows=" << ctx.getWindows().size() << "\n";
  std::cout << "sandbox running... (close all windows to exit)\n";
  ctx.runMainLoop();
  // Clean up pipeline(s) allocated at runtime
  if (triPipeline) {
    // Detach from window first
    if (win1) win1->pipeline = nullptr;
    ctx.destroyPipeline(triPipeline);
  }

  ctx.shutdown();
  return 0;
}
