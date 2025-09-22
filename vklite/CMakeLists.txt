add_library(vklite
    src/vklite.cpp
)

target_include_directories(vklite
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

find_package(Vulkan REQUIRED)
target_link_libraries(vklite PUBLIC Vulkan::Vulkan)

# Platform identification macros for use in headers / PCHs
if(WIN32)
    target_compile_definitions(vklite PRIVATE VKLITE_PLAT_WINDOWS=1)
elseif(APPLE)
    target_compile_definitions(vklite PRIVATE VKLITE_PLAT_MAC=1)
elseif(UNIX)
    target_compile_definitions(vklite PRIVATE VKLITE_PLAT_LINUX=1)
endif()

target_compile_features(vklite PUBLIC cxx_std_17)

if(WIN32)
    target_compile_definitions(vklite PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(vklite PRIVATE VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
    target_compile_definitions(vklite PRIVATE VK_USE_PLATFORM_XCB_KHR)
endif()
