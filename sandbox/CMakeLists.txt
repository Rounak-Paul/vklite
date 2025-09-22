add_executable(sandbox src/main.cpp)

target_link_libraries(sandbox PRIVATE vklite)

target_compile_features(sandbox PRIVATE cxx_std_17)
