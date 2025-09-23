---
title: Vulkan Instance Creation
---

# Creating a Vulkan Instance

The first step in any Vulkan application is to create a Vulkan instance. The instance represents the connection between your application and the Vulkan library. It is required before you can query for physical devices, create surfaces, or perform any rendering operations.

This chapter will guide you through the process of creating and destroying a Vulkan instance in C++ using the Vulkan SDK.

---

## Update the Header

Add the Vulkan headers and a member to hold the instance handle in `vklite.h`:

```cpp
#include <vulkan/vulkan.h>

struct Context {
    VkInstance instance = VK_NULL_HANDLE;
    // ...existing code...
};
```

---


## Create the Instance in C++

In `vklite.cpp`, update the `initialize` and `shutdown` methods to:

1. Query the Vulkan loader for the maximum supported API version and require at least 1.3.
2. Query required instance extensions from GLFW, and add the portability extension for macOS.
3. Pass these extensions to the Vulkan instance creation struct.

```cpp
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <cstring>

// ...

// Query Vulkan loader for supported API version
uint32_t apiVersion = 0;
if (vkEnumerateInstanceVersion) {
    vkEnumerateInstanceVersion(&apiVersion);
} else {
    apiVersion = VK_API_VERSION_1_0;
}
if (apiVersion < VK_API_VERSION_1_3) {
    std::cerr << "Vulkan 1.3 or higher is required!" << std::endl;
    return false;
}

// Query required extensions from GLFW
if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW!" << std::endl;
    return false;
}
uint32_t glfwExtensionCount = 0;
const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#if defined(VKLITE_PLAT_MAC)
// Add portability enumeration extension for macOS
extensions.push_back("VK_KHR_portability_enumeration");
#endif

VkApplicationInfo appInfo{};
appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
appInfo.pApplicationName = appName.c_str();
appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
appInfo.pEngineName = "vklite";
appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
appInfo.apiVersion = VK_API_VERSION_1_3;

VkInstanceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
createInfo.pApplicationInfo = &appInfo;
createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
createInfo.ppEnabledExtensionNames = extensions.data();
#if defined(VKLITE_PLAT_MAC)
createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
if (result != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance!\n";
    instance = VK_NULL_HANDLE;
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

## Minimal Working Example

After these changes, your `Context` will create and destroy a Vulkan instance. You can now build and run your project to verify that instance creation succeeds (no validation layers or extensions are enabled yet).

---

**Next** [Window Creation](Window_Creation.md)
