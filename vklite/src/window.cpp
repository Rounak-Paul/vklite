// window.cpp - window management implementation for vklite
#include "vklite.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

namespace vklite {

Window* Context::createWindow(int width, int height, const std::string& title) {
  GLFWwindow* win = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!win) {
    // Creation failed; surface-level errors can be retrieved by the application
    // via the GLFW error callback if needed. Return nullptr to indicate failure.
    return nullptr;
  }
  auto w = std::make_unique<Window>();
  w->handle = win;
  w->width = width;
  w->height = height;
  w->title = title;
  windows.push_back(std::move(w));
  return windows.back().get();
}

void Context::destroyWindow(Window* window) {
  if (!window || !window->handle) return;
  GLFWwindow* gw = static_cast<GLFWwindow*>(window->handle);
  glfwDestroyWindow(gw);
  window->handle = nullptr;
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
    pollEvents();
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

} // namespace vklite