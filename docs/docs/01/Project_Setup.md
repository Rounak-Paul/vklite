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
│   └── src/
│       └── vklite.cpp
├── sandbox/
│   ├── CMakeLists.txt
│   └── src/
│       └── main.cpp
```

This separates the core renderer (`vklite`) from example or test applications (`sandbox`).

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
    // Initializes Vulkan instance, queries loader version and prepares GLFW.
    // See the Instance Creation chapter for details on instance flags and
    // required extensions.
    return true;
}

void Context::shutdown(){
    // Clean up Vulkan and windowing resources.
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

In your `vklite/CMakeLists.txt`, find and link the Vulkan library:

```cmake
find_package(Vulkan REQUIRED)

add_library(vklite
	src/vklite.cpp
	include/vklite.h
)

target_include_directories(vklite PUBLIC include)
target_link_libraries(vklite PUBLIC Vulkan::Vulkan)
set_target_properties(vklite PROPERTIES CXX_STANDARD 20)
```

This ensures your renderer links against the Vulkan SDK and uses modern C++.

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

**Next** [Instance Creation](Instance_Creation.md)

