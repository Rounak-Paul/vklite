
# Add vendor submodules
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/tinyobjloader)
add_subdirectory(vendor/vma)

find_package(Vulkan REQUIRED)
find_package(shaderc QUIET)
if(NOT shaderc_FOUND)
    # Try pkg-config as a fallback
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PC_SHADERC QUIET shaderc)
        if(PC_SHADERC_FOUND)
            # pkg-config returned link flags in PC_SHADERC_LIBRARIES (e.g. -L... -lshaderc_shared)
            # Save them for later rather than trying to use them as an IMPORTED_LOCATION.
            set(SHADERC_LIBS ${PC_SHADERC_LIBRARIES})
            set(SHADERC_INCLUDE_DIR ${PC_SHADERC_INCLUDE_DIRS})
            set(shaderc_FOUND TRUE)
        endif()
    endif()
endif()

if(NOT shaderc_FOUND)
    # Try to find headers and library manually
    find_path(SHADERC_INCLUDE_DIR NAMES shaderc/shaderc.hpp)
    find_library(SHADERC_LIB NAMES shaderc shaderc_shared shaderc_static)
    if(SHADERC_INCLUDE_DIR AND SHADERC_LIB)
        add_library(shaderc::shaderc UNKNOWN IMPORTED)
        set_target_properties(shaderc::shaderc PROPERTIES
            IMPORTED_LOCATION "${SHADERC_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${SHADERC_INCLUDE_DIR}")
        set(shaderc_FOUND TRUE)
    endif()
endif()

add_library(vklite
    src/vklite.cpp
    src/window.cpp
    src/pipeline.cpp
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

if(TARGET shaderc::shaderc)
    target_link_libraries(vklite PUBLIC shaderc::shaderc)
    target_compile_definitions(vklite PRIVATE VKLITE_USE_SHADERC=1)
elseif(shaderc_FOUND)
    # shaderc was detected but no imported target is available; try to use the found library via variable
    if(DEFINED SHADERC_LIB)
        add_library(shaderc::shaderc UNKNOWN IMPORTED)
        set_target_properties(shaderc::shaderc PROPERTIES IMPORTED_LOCATION "${SHADERC_LIB}" INTERFACE_INCLUDE_DIRECTORIES "${SHADERC_INCLUDE_DIR}")
        target_link_libraries(vklite PUBLIC shaderc::shaderc)
        target_compile_definitions(vklite PRIVATE VKLITE_USE_SHADERC=1)
    elseif(DEFINED SHADERC_LIBS)
        # pkg-config provided linker flags; try to construct an absolute library path
        if(DEFINED PC_SHADERC_LIBDIR AND DEFINED PC_SHADERC_LIBRARIES)
            # PC_SHADERC_LIBDIR may be a single path like /usr/local/lib
            set(_pc_libdir ${PC_SHADERC_LIBDIR})
            set(_pc_libname ${PC_SHADERC_LIBRARIES})
            # Attempt macOS dylib naming first
            set(_candidate_lib "${_pc_libdir}/lib${_pc_libname}.dylib")
            if(EXISTS "${_candidate_lib}")
                add_library(shaderc::shaderc UNKNOWN IMPORTED)
                set_target_properties(shaderc::shaderc PROPERTIES IMPORTED_LOCATION "${_candidate_lib}" INTERFACE_INCLUDE_DIRECTORIES "${SHADERC_INCLUDE_DIR}")
                target_link_libraries(vklite PUBLIC shaderc::shaderc)
                target_compile_definitions(vklite PRIVATE VKLITE_USE_SHADERC=1)
            else()
                # Fallback: apply raw pkg-config flags directly
                target_link_libraries(vklite PUBLIC ${SHADERC_LIBS})
                target_include_directories(vklite PUBLIC ${SHADERC_INCLUDE_DIR})
                target_compile_definitions(vklite PRIVATE VKLITE_USE_SHADERC=1)
            endif()
        else()
            target_link_libraries(vklite PUBLIC ${SHADERC_LIBS})
            target_include_directories(vklite PUBLIC ${SHADERC_INCLUDE_DIR})
            target_compile_definitions(vklite PRIVATE VKLITE_USE_SHADERC=1)
        endif()
    else()
        message(WARNING "shaderc found but no usable target; falling back to external glslangValidator invocation.")
        target_compile_definitions(vklite PRIVATE VKLITE_USE_GLSLANG_FALLBACK=1)
    endif()
else()
    message(WARNING "shaderc not found; falling back to external glslangValidator invocation. To use shaderc, install shaderc and re-run CMake.")
    target_compile_definitions(vklite PRIVATE VKLITE_USE_GLSLANG_FALLBACK=1)
endif()

target_compile_features(vklite PUBLIC cxx_std_17)

# Platform identification macros for use in headers / PCHs
if(WIN32)
    target_compile_definitions(vklite PRIVATE VKLITE_PLAT_WINDOWS=1)
elseif(APPLE)
    target_compile_definitions(vklite PRIVATE VKLITE_PLAT_MAC=1)
elseif(UNIX)
    target_compile_definitions(vklite PRIVATE VKLITE_PLAT_LINUX=1)
endif()

if(WIN32)
    target_compile_definitions(vklite PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(vklite PRIVATE VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
    target_compile_definitions(vklite PRIVATE VK_USE_PLATFORM_XCB_KHR)
endif()
