---
title: Project Setup
---

# Setting Up the Project Structure

With the Vulkan SDK installed and verified (see previous chapter), we are ready to set up our C++ project for Vulkan development. This chapter will guide you through creating a clean, modular project structure using CMake, and configuring it to build against the Vulkan SDK.

---

## Create the Folder Structure


Organize your project for clarity and scalability. A recommended structure is:

```text
project_root/
├── CMakeLists.txt
├── vklite/
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── vklite.h
│   ├── src/
│   │   └── vklite.cpp
│   └── vendor/
│       ├── entt/
│       ├── glfw/
│       ├── glm/
│       ├── imgui/
│       ├── tinyobjloader/
│       └── vma/
├── sandbox/
│   ├── CMakeLists.txt
│   └── src/
│       └── main.cpp
```

This separates the core renderer (`vklite`) from example or test applications (`sandbox`), and places all third-party dependencies in the `vendor` folder as submodules.

## Source Code for Starter Files

Below are the minimal source files to get your project building and running. Copy these into the appropriate locations:

### `vklite/include/vklite.h`
```cpp
#pragma once

#include <string>

// Platform macros provided by the build system:
// - VKLITE_PLAT_WINDOWS (windows)
// - VKLITE_PLAT_LINUX   (linux / unix)
// - VKLITE_PLAT_MAC     (apple / macos)

namespace vklite {

struct Context {
	// Initialize creates the Vulkan instance and prepares internal state.
	// Returns true on success, false on failure.
	bool initialize(const std::string &appName = "vklite-app");

	// Shutdown cleans up Vulkan objects and internal resources.
	void shutdown();
};

} // namespace vklite
```

### `vklite/src/vklite.cpp`
```cpp
#include "vklite.h"
#include <iostream>

namespace vklite {

bool Context::initialize(const std::string &appName){
	std::cout << "vklite: initialize for " << appName;
#if defined(VKLITE_PLAT_WINDOWS)
	std::cout << " [platform: windows]";
#elif defined(VKLITE_PLAT_MAC)
	std::cout << " [platform: macos]";
#elif defined(VKLITE_PLAT_LINUX)
	std::cout << " [platform: linux]";
#endif
	std::cout << "\n";
	return true;
}

void Context::shutdown(){
	std::cout << "vklite: shutdown\n";
}

} // namespace vklite
```

### `sandbox/src/main.cpp`
```cpp
#include "vklite.h"
#include <iostream>

int main(){
	vklite::Context ctx;
	if(!ctx.initialize("sandbox")){
		std::cerr << "Failed to initialize vklite\n";
		return 1;
	}

	std::cout << "sandbox running...\n";

	ctx.shutdown();
	return 0;
}
```

---

## Initialize CMake Build System

At the root, create a `CMakeLists.txt` to manage the overall build:

```cmake
cmake_minimum_required(VERSION 3.16)
project(vklite LANGUAGES CXX)

add_subdirectory(vklite)
add_subdirectory(sandbox)
```

Each subdirectory (`vklite`, `sandbox`) should have its own `CMakeLists.txt`.

---

## Configure Vulkan SDK in CMake

target_include_directories(vklite PUBLIC include)
target_link_libraries(vklite PUBLIC Vulkan::Vulkan)

In your `vklite/CMakeLists.txt`, add each vendor as a subdirectory and link their targets to your library:

```cmake
# Add vendor submodules
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/tinyobjloader)
add_subdirectory(vendor/vma)

find_package(Vulkan REQUIRED)

add_library(vklite
	src/vklite.cpp
)

target_include_directories(vklite
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(vklite
	PUBLIC
		Vulkan::Vulkan
		glfw
		glm-header-only
		EnTT
		ImGui
		tinyobjloader
		GPUOpen::VulkanMemoryAllocator
)
```

This ensures your renderer links against the Vulkan SDK and all third-party dependencies, using modern C++.

---

## Add an Example Application

In `sandbox/CMakeLists.txt`:

```cmake
add_executable(sandbox src/main.cpp)
target_link_libraries(sandbox PRIVATE vklite)
set_target_properties(sandbox PROPERTIES CXX_STANDARD 20)
```

This builds a simple application that uses your renderer.

---

## Build the Project

From the project root, configure and build:

```sh
cmake -S . -B build
cmake --build build
```

If everything is set up correctly, you should see both the `vklite` library and the `sandbox` executable built in the `build/` directory.

---

## Next Steps


You are now ready to begin implementing Vulkan features in your renderer. The next chapter will guide you through creating the Vulkan instance, the first step in any Vulkan application.

**Next:** [Instance Creation](Instance_Creation.md)

