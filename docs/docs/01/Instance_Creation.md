---
title: Vulkan Instance Creation
---

# Creating a Vulkan Instance

The first step in any Vulkan application is to create a Vulkan instance. The instance represents the connection between your application and the Vulkan library. It is required before you can query for physical devices, create surfaces, or perform any rendering operations.

This chapter will guide you through the process of creating and destroying a Vulkan instance in C++ using the Vulkan SDK.

---

## 1. Update the Header

Add the Vulkan headers and a member to hold the instance handle in `vklite.h`:

```cpp
#include <vulkan/vulkan.h>

struct Context {
    VkInstance instance = VK_NULL_HANDLE;
    // ...existing code...
};
```

---

## 2. Create the Instance in C++

In `vklite.cpp`, update the `initialize` and `shutdown` methods to create and destroy the Vulkan instance:

```cpp
VkApplicationInfo appInfo{};
appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
appInfo.pApplicationName = appName.c_str();
appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
appInfo.pEngineName = "vklite";
appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
appInfo.apiVersion = VK_API_VERSION_1_3;

VkInstanceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
createInfo.pApplicationInfo = &appInfo;

VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
if (result != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance!\n";
    return false;
}
```

And in `shutdown`:

```cpp
if (instance != VK_NULL_HANDLE) {
    vkDestroyInstance(instance, nullptr);
    instance = VK_NULL_HANDLE;
}
```

---

## 3. Minimal Working Example

After these changes, your `Context` will create and destroy a Vulkan instance. You can now build and run your project to verify that instance creation succeeds (no validation layers or extensions are enabled yet).

---

**Next:** [Render Loop]()
