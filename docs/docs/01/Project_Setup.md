---
title: Project Setup
---

This is the canonical Project Setup for vklite. It provides copy-pasteable CMake files and minimal example sources to build the `vklite` library and a small `sandbox` demo on Windows, Linux and macOS. Vulkan is required; please install the LunarG (or vendor) SDK and set the `VULKAN_SDK` environment variable.

Prerequisites

- CMake >= 3.15
- A C++17-capable compiler
- Vulkan SDK installed and VULKAN_SDK set

Repository layout

```
vklite/
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
├── CMakeLists.txt
└── README.md
```

Top-level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(vklite VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(VKLITE_BUILD_SANDBOX "Build sandbox demo" ON)
add_subdirectory(vklite)
if(VKLITE_BUILD_SANDBOX)
	add_subdirectory(sandbox)
endif()
```

vklite/CMakeLists.txt

```cmake
add_library(vklite src/vklite.cpp)
target_include_directories(vklite PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
find_package(Vulkan REQUIRED)
target_link_libraries(vklite PUBLIC Vulkan::Vulkan)

if(WIN32)
	target_compile_definitions(vklite PRIVATE VKLITE_PLAT_WINDOWS=1 VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
	target_compile_definitions(vklite PRIVATE VKLITE_PLAT_MAC=1 VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
	target_compile_definitions(vklite PRIVATE VKLITE_PLAT_LINUX=1 VK_USE_PLATFORM_XCB_KHR)
endif()

target_compile_features(vklite PUBLIC cxx_std_17)
```

vklite/include/vklite.h

```cpp
#pragma once
#include <string>
namespace vklite {
struct Context { bool initialize(const std::string &appName = "vklite-app"); void shutdown(); };
}
```

vklite/src/vklite.cpp

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

void Context::shutdown(){ std::cout << "vklite: shutdown\n"; }
}
```

sandbox/CMakeLists.txt

```cmake
add_executable(sandbox src/main.cpp)
target_link_libraries(sandbox PRIVATE vklite)
target_compile_features(sandbox PRIVATE cxx_std_17)
```

sandbox/src/main.cpp

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

Build (UNIX/macOS)

```sh
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
```

Windows (Visual Studio)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Troubleshooting

- "Vulkan SDK not found": ensure `VULKAN_SDK` is set and the SDK is installed.
- macOS: MoltenVK must be present in the SDK.

Next steps: implement full Vulkan initialization in `vklite/src/` and document it on follow-up pages.


## Project setup (Windows, Linux, macOS)

This guide shows a simple, cross-platform CMake setup for building VkLite as a library and a small `sandbox` executable that links to it. The project uses a single CMake workflow that auto-detects the platform and links the right system libraries.

Goals / contract
- Inputs: Vulkan SDK installed and VULKAN_SDK environment variable set; CMake (>=3.15); a C++17-capable compiler.
- Outputs: a `vklite` library and a `sandbox` executable that links to `vklite` and Vulkan.
- Error modes: missing Vulkan SDK, missing compiler, generator issues on Windows. The guide shows how to resolve these.

Directory layout

Recommended repository layout (relative to the repo root):

```
vklite/                # project root (this repo)
---
title: Project Setup
---

This page provides exact, copy-pasteable files and clear instructions to set up VkLite on Windows, Linux and macOS. Vulkan is required — install the LunarG SDK (or vendor SDK) and ensure CMake can find it (commonly via the `VULKAN_SDK` environment variable).

Overview
- Required: Vulkan SDK (headers, libraries and validation layers), CMake >= 3.15, and a C++17-capable compiler.
- Output: a `vklite` static/shared library and a `sandbox` executable.

---
title: Project Setup
---

This page contains a single, canonical copy of the project layout and all example files needed to build VkLite and the sandbox demo on Windows, Linux and macOS.

Prerequisites
- CMake >= 3.15
- A C++17-capable compiler
- Vulkan SDK (LunarG or vendor SDK). Ensure `VULKAN_SDK` points at the SDK root.

Project layout

```
vklite/
├── vklite/
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── vklite.h
│   └── src/
---
title: Project Setup
---

This document is the canonical Project Setup for vklite. It contains the minimal files and example sources to build the library and a sandbox demo on Windows, Linux and macOS. Vulkan is required; ensure the Vulkan SDK is installed and `VULKAN_SDK` points to it.

Prerequisites
- CMake >= 3.15
- A C++17-capable compiler
- Vulkan SDK (LunarG or vendor SDK)

Layout

```
vklite/
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
├── CMakeLists.txt
└── README.md
```

Top-level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(vklite VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(VKLITE_BUILD_SANDBOX "Build sandbox demo" ON)
add_subdirectory(vklite)
if(VKLITE_BUILD_SANDBOX)
	add_subdirectory(sandbox)
endif()
```

vklite/CMakeLists.txt

```cmake
add_library(vklite src/vklite.cpp)
target_include_directories(vklite PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
find_package(Vulkan REQUIRED)
target_link_libraries(vklite PUBLIC Vulkan::Vulkan)
if(WIN32)
	target_compile_definitions(vklite PRIVATE VKLITE_PLAT_WINDOWS=1 VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
	target_compile_definitions(vklite PRIVATE VKLITE_PLAT_MAC=1 VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
	target_compile_definitions(vklite PRIVATE VKLITE_PLAT_LINUX=1 VK_USE_PLATFORM_XCB_KHR)
endif()
target_compile_features(vklite PUBLIC cxx_std_17)
```

vklite/include/vklite.h

```cpp
#pragma once
#include <string>
namespace vklite {
struct Context { bool initialize(const std::string &appName = "vklite-app"); void shutdown(); };
}
```

vklite/src/vklite.cpp

```cpp
#include "vklite.h"
#include <iostream>
namespace vklite {
bool Context::initialize(const std::string &appName){ std::cout << "vklite: initialize for " << appName; #if defined(VKLITE_PLAT_WINDOWS) std::cout << " [platform: windows]"; #elif defined(VKLITE_PLAT_MAC) std::cout << " [platform: macos]"; #elif defined(VKLITE_PLAT_LINUX) std::cout << " [platform: linux]"; #endif std::cout << "\n"; return true; }
void Context::shutdown(){ std::cout << "vklite: shutdown\n"; }
}
```

sandbox/CMakeLists.txt

```cmake
add_executable(sandbox src/main.cpp)
target_link_libraries(sandbox PRIVATE vklite)
target_compile_features(sandbox PRIVATE cxx_std_17)
```

sandbox/src/main.cpp

```cpp
#include "vklite.h"
#include <iostream>
int main(){ vklite::Context ctx; if(!ctx.initialize("sandbox")){ std::cerr << "Failed to initialize vklite\n"; return 1; } std::cout << "sandbox running...\n"; ctx.shutdown(); return 0; }
```

Build (UNIX/macOS)

```sh
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
```

Windows (Visual Studio)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Troubleshooting

- "Vulkan SDK not found": ensure `VULKAN_SDK` is set and the SDK is installed.
- macOS: MoltenVK must be present in the SDK.

Next steps: implement full Vulkan initialization in `vklite/src/` and document it on follow-up pages.