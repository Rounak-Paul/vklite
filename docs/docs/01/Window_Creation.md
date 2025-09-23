---
title: Creating Windows and the Application Loop
---

# Creating Windows and the Application Loop

With Vulkan instance creation complete, the next step is to create one or more application windows and set up the main loop. In VkLite, window management is fully abstracted: you can create as many windows as you like, and each can later be associated with its own swapchain and renderer.

This chapter demonstrates how to use the `vklite::Context` API to create, manage, and destroy windows, and how to structure a basic application loop.

---

## Creating Windows

After initializing the `Context`, you can create windows using the `createWindow` method. Each window is represented by a `Window*` handle managed by the context.

```cpp
vklite::Context ctx;
if (!ctx.initialize("sandbox")) {
    // handle error
}

// Create two windows
auto* win1 = ctx.createWindow(800, 600, "VkLite Window 1");
auto* win2 = ctx.createWindow(640, 480, "VkLite Window 2");
if (!win1 || !win2) {
    // handle error
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

## Next Steps

You can now create and manage multiple windows in your application. The next step is to create a Vulkan surface and swapchain for each window, and begin rendering.
