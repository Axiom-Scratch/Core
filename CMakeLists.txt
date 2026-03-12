cmake_minimum_required(VERSION 3.20)

project(CoreRuntime LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CORE_BUILD_EXAMPLES "Build examples" ON)
option(CORE_ENABLE_WARNINGS "Enable warnings" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(external/entt)
add_subdirectory(external/glm)
set(GLAD_API "gl=3.3" CACHE STRING "GLAD API configuration" FORCE)
set(GLAD_PROFILE "core" CACHE STRING "GLAD OpenGL profile" FORCE)
set(GLAD_GENERATOR "c" CACHE STRING "GLAD generator" FORCE)
set(GLAD_REPRODUCIBLE ON CACHE BOOL "GLAD reproducible build" FORCE)
add_subdirectory(external/glad)
add_subdirectory(Core)

if(CORE_ENABLE_WARNINGS)
    target_compile_options(Core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
