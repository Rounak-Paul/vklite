---
title: Creating Windows and the Application Loop
---

# Creating Windows and the Application Loop

With Vulkan instance creation complete, the next step is to create one or more application windows and set up the main loop. In VkLite, window management is fully abstracted: you can create as many windows as you like, and each can later be associated with its own swapchain and renderer.

This chapter demonstrates how to use the `vklite::Context` API to create, manage, and destroy windows, and how to structure a basic application loop.

---

## Creating Windows (self-contained implementation)

This section contains full header and implementation code you can copy directly into your project. The goal is that a reader can follow the doc and build the example without opening the repository source files.

Place the following header in `vklite/include/window.h`:

```cpp
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
```

Place the following implementation in `vklite/src/window.cpp`:

```cpp
// window.cpp - window management implementation for vklite
#include "vklite.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <vector>

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
```

### Example application

Copy this minimal `sandbox/src/main.cpp` to see the windowing example in action:

```cpp
#include "vklite.h"
#include <iostream>

int main() {
        vklite::Context ctx;
        if (!ctx.initialize("sandbox")) {
                std::cerr << "Failed to initialize vklite\n";
                return 1;
        }

        auto* win1 = ctx.createWindow(800, 600, "VkLite Window 1");
        auto* win2 = ctx.createWindow(640, 480, "VkLite Window 2");
        if (!win1 || !win2) {
                std::cerr << "Failed to create one or more windows\n";
                ctx.shutdown();
                return 1;
        }

        // Application is running; close windows to exit the loop.
        ctx.runMainLoop();

        ctx.shutdown();
        return 0;
}
```

---

## The Application Loop

VkLite centralizes the application loop for you. The recommended approach is to call `ctx.runMainLoop()` which polls events and will return when all windows are closed. This keeps your application code concise and avoids accidental misuse of GLFW.

---

## Cleanup

When the loop exits, call `ctx.shutdown()` to destroy all windows and release resources:

```cpp
ctx.shutdown();
```

---

## Full Example

```cpp
#include "vklite.h"
#include <iostream>

int main() {
    vklite::Context ctx;
    if (!ctx.initialize("sandbox")) {
        std::cerr << "Failed to initialize vklite\n";
        return 1;
    }

    auto* win1 = ctx.createWindow(800, 600, "VkLite Window 1");
    auto* win2 = ctx.createWindow(640, 480, "VkLite Window 2");
    if (!win1 || !win2) {
        std::cerr << "Failed to create one or more windows\n";
        ctx.shutdown();
        return 1;
    }

    // Application is running; close windows to exit the loop.

    ctx.runMainLoop();

    ctx.shutdown();
    return 0;
}
```

---

## Advanced topics & where the code lives

This page focuses on window creation and the application loop using the `vklite` API. If you are interested in the Vulkan-side steps (surfaces, swapchains, render passes), those are advanced topics that are intentionally documented separately and implemented in their own modules.

Where to look in the source:

- `vklite/include/window.h` — the public `Window` struct and API types.
- `vklite/src/window.cpp` — the implementation of `Context::createWindow`, `destroyWindow`, `isWindowOpen`, `pollEvents`, and `runMainLoop`.

Recommended next docs:

- Instance creation and required instance extensions: [Instance_Creation.md](Instance_Creation.md)
- Swapchain & rendering (planned): a separate guide that walks through creating `VkSurfaceKHR`, choosing formats/present modes, creating `VkSwapchainKHR`, and setting up a minimal render pass.

If you'd like that swapchain guide authored here, I can create a dedicated `Swapchain.md` that uses your source examples as the canonical code for the tutorial.
