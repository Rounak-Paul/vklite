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
