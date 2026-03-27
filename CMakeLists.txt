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



# not include this its diffrent 
cmake_minimum_required(VERSION 3.20)

project(procedural_planet LANGUAGES C CXX)

function(axiom_option NAME DEFAULT DESCRIPTION)
    option(${NAME} "${DESCRIPTION}" ${DEFAULT})
endfunction()

function(axiom_define_toggle NAME)
    if(${NAME})
        add_compile_definitions(${NAME}=1)
    else()
        add_compile_definitions(${NAME}=0)
    endif()
endfunction()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CONFIGURATION_TYPES)
    string(JOIN ", " AXIOM_AVAILABLE_CONFIGS ${CMAKE_CONFIGURATION_TYPES})
    set(AXIOM_BUILD_TYPE_DISPLAY "Multi-config [${AXIOM_AVAILABLE_CONFIGS}]")
else()
    if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    endif()

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug
        Release
        RelWithDebInfo
        MinSizeRel
    )
    set(AXIOM_BUILD_TYPE_DISPLAY "${CMAKE_BUILD_TYPE}")
endif()

axiom_option(AXIOM_ENABLE_ASSERTS ON "Enable engine assertions")
axiom_option(AXIOM_ENABLE_LOGGING ON "Enable engine logging")

axiom_option(AXIOM_ENABLE_ASAN ON "Enable AddressSanitizer")
axiom_option(AXIOM_ENABLE_UBSAN ON "Enable UndefinedBehaviorSanitizer")
axiom_option(AXIOM_ENABLE_TSAN OFF "Enable ThreadSanitizer")
axiom_option(AXIOM_ENABLE_LSAN OFF "Enable LeakSanitizer")

axiom_option(AXIOM_ENABLE_PROFILING OFF "Enable profiler instrumentation")
axiom_option(AXIOM_ENABLE_TRACY ON "Enable Tracy profiler")
axiom_option(AXIOM_ENABLE_PERF OFF "Enable perf profiling")
axiom_option(AXIOM_ENABLE_DEBUG_SYMBOLS ON "Enable debug symbols")
axiom_option(AXIOM_ENABLE_OPTIMIZATION OFF "Enable compiler optimizations")
axiom_option(AXIOM_ENABLE_LTO OFF "Enable link-time optimization")

set(AXIOM_COMPILER_FAMILY "Unknown")
set(AXIOM_COMPILER_IS_GCC OFF)
set(AXIOM_COMPILER_IS_CLANG OFF)
set(AXIOM_COMPILER_IS_MSVC OFF)

if(MSVC)
    set(AXIOM_COMPILER_FAMILY "MSVC")
    set(AXIOM_COMPILER_IS_MSVC ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(AXIOM_COMPILER_FAMILY "GCC")
    set(AXIOM_COMPILER_IS_GCC ON)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(AXIOM_COMPILER_FAMILY "Clang")
    set(AXIOM_COMPILER_IS_CLANG ON)
else()
    message(FATAL_ERROR
        "Unsupported compiler '${CMAKE_CXX_COMPILER_ID}'. "
        "Supported compilers: GCC, Clang, and MSVC."
    )
endif()

set(AXIOM_COMPILE_FLAGS "")
set(AXIOM_LINK_FLAGS "")
set(AXIOM_SANITIZER_COMPONENTS "")

if(AXIOM_ENABLE_DEBUG_SYMBOLS)
    if(AXIOM_COMPILER_IS_GCC)
        list(APPEND AXIOM_COMPILE_FLAGS -g3 -ggdb)
    elseif(AXIOM_COMPILER_IS_CLANG)
        list(APPEND AXIOM_COMPILE_FLAGS -g -glldb)
    elseif(AXIOM_COMPILER_IS_MSVC)
        list(APPEND AXIOM_COMPILE_FLAGS /Zi)
    endif()
endif()

if(AXIOM_COMPILER_IS_GCC OR AXIOM_COMPILER_IS_CLANG)
    list(APPEND AXIOM_COMPILE_FLAGS -fno-omit-frame-pointer)
endif()

if(AXIOM_ENABLE_OPTIMIZATION)
    if(AXIOM_COMPILER_IS_MSVC)
        list(APPEND AXIOM_COMPILE_FLAGS /O2)
    else()
        list(APPEND AXIOM_COMPILE_FLAGS -O2)
    endif()
else()
    if(AXIOM_COMPILER_IS_MSVC)
        list(APPEND AXIOM_COMPILE_FLAGS /Od)
    else()
        list(APPEND AXIOM_COMPILE_FLAGS -O0)
    endif()
endif()

if(AXIOM_ENABLE_ASAN)
    list(APPEND AXIOM_SANITIZER_COMPONENTS address)
endif()

if(AXIOM_ENABLE_UBSAN)
    list(APPEND AXIOM_SANITIZER_COMPONENTS undefined)
endif()

if(AXIOM_SANITIZER_COMPONENTS)
    if(AXIOM_COMPILER_IS_MSVC)
        message(FATAL_ERROR
            "Sanitizers are only supported for GCC and Clang in this build configuration."
        )
    endif()

    list(JOIN AXIOM_SANITIZER_COMPONENTS "," AXIOM_SANITIZER_FLAG_SUFFIX)
    list(APPEND AXIOM_COMPILE_FLAGS "-fsanitize=${AXIOM_SANITIZER_FLAG_SUFFIX}")
    list(APPEND AXIOM_LINK_FLAGS "-fsanitize=${AXIOM_SANITIZER_FLAG_SUFFIX}")
endif()

if(AXIOM_ENABLE_PROFILING)
    if(AXIOM_COMPILER_IS_MSVC)
        message(FATAL_ERROR
            "AXIOM_ENABLE_PROFILING requires GCC or Clang because it uses -pg."
        )
    endif()

    list(APPEND AXIOM_COMPILE_FLAGS -pg)
    list(APPEND AXIOM_LINK_FLAGS -pg)
endif()

if(AXIOM_ENABLE_PERF)
    if(AXIOM_COMPILER_IS_GCC OR AXIOM_COMPILER_IS_CLANG)
        list(APPEND AXIOM_COMPILE_FLAGS -g -fno-omit-frame-pointer)
        list(APPEND AXIOM_LINK_FLAGS -rdynamic)
    endif()
endif()

if(AXIOM_ENABLE_LTO)
    if(AXIOM_COMPILER_IS_MSVC)
        list(APPEND AXIOM_COMPILE_FLAGS /GL)
        list(APPEND AXIOM_LINK_FLAGS /LTCG)
    else()
        list(APPEND AXIOM_COMPILE_FLAGS -flto)
        list(APPEND AXIOM_LINK_FLAGS -flto)
    endif()
endif()

include(cmake/compiler_warnings.cmake)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(LINK_PATH "${CMAKE_SOURCE_DIR}/compile_commands.json")

    if(EXISTS "${LINK_PATH}" OR IS_SYMLINK "${LINK_PATH}")
        file(REMOVE "${LINK_PATH}")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_BINARY_DIR}/compile_commands.json
                ${LINK_PATH}
    )
endif()

message(STATUS "---- Dependencies ----")

add_subdirectory(external/glfw)

add_subdirectory(external/glad/cmake)
glad_add_library(glad REPRODUCIBLE API gl:core=3.3)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)

add_subdirectory(external/spdlog)

if(AXIOM_ENABLE_TRACY)
    add_subdirectory(external/tracy)
    add_compile_definitions(TRACY_ENABLE)
endif()

if(AXIOM_COMPILE_FLAGS)
    add_compile_options(${AXIOM_COMPILE_FLAGS})
endif()

if(AXIOM_LINK_FLAGS)
    add_link_options(${AXIOM_LINK_FLAGS})
endif()

axiom_define_toggle(AXIOM_ENABLE_ASSERTS)
axiom_define_toggle(AXIOM_ENABLE_LOGGING)

add_subdirectory(Engine)

if(AXIOM_ENABLE_TRACY)
    target_sources(Engine PRIVATE
        ${CMAKE_SOURCE_DIR}/external/tracy/public/TracyClient.cpp
    )
    target_link_libraries(Engine PRIVATE Tracy::TracyClient)
endif()

if(AXIOM_COMPILE_FLAGS)
    string(JOIN " " AXIOM_COMPILE_FLAGS_STRING ${AXIOM_COMPILE_FLAGS})
else()
    set(AXIOM_COMPILE_FLAGS_STRING "<none>")
endif()

if(AXIOM_LINK_FLAGS)
    string(JOIN " " AXIOM_LINK_FLAGS_STRING ${AXIOM_LINK_FLAGS})
else()
    set(AXIOM_LINK_FLAGS_STRING "<none>")
endif()

if(AXIOM_SANITIZER_COMPONENTS)
    string(JOIN ", " AXIOM_SANITIZERS_STRING ${AXIOM_SANITIZER_COMPONENTS})
else()
    set(AXIOM_SANITIZERS_STRING "<none>")
endif()

message(STATUS "========== Axiom Build Configuration ==========")
message(STATUS "Compiler Path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler Family: ${AXIOM_COMPILER_FAMILY}")
message(STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build Type: ${AXIOM_BUILD_TYPE_DISPLAY}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "AXIOM_ENABLE_ASSERTS: ${AXIOM_ENABLE_ASSERTS}")
message(STATUS "AXIOM_ENABLE_LOGGING: ${AXIOM_ENABLE_LOGGING}")
message(STATUS "AXIOM_ENABLE_ASAN: ${AXIOM_ENABLE_ASAN}")
message(STATUS "AXIOM_ENABLE_UBSAN: ${AXIOM_ENABLE_UBSAN}")
message(STATUS "AXIOM_ENABLE_TSAN: ${AXIOM_ENABLE_TSAN}")
message(STATUS "AXIOM_ENABLE_LSAN: ${AXIOM_ENABLE_LSAN}")
message(STATUS "AXIOM_ENABLE_PROFILING: ${AXIOM_ENABLE_PROFILING}")
message(STATUS "AXIOM_ENABLE_TRACY: ${AXIOM_ENABLE_TRACY}")
message(STATUS "AXIOM_ENABLE_PERF: ${AXIOM_ENABLE_PERF}")
message(STATUS "AXIOM_ENABLE_DEBUG_SYMBOLS: ${AXIOM_ENABLE_DEBUG_SYMBOLS}")
message(STATUS "AXIOM_ENABLE_OPTIMIZATION: ${AXIOM_ENABLE_OPTIMIZATION}")
message(STATUS "AXIOM_ENABLE_LTO: ${AXIOM_ENABLE_LTO}")
message(STATUS "Active Sanitizers: ${AXIOM_SANITIZERS_STRING}")
message(STATUS "Active Compile Flags: ${AXIOM_COMPILE_FLAGS_STRING}")
message(STATUS "Active Link Flags: ${AXIOM_LINK_FLAGS_STRING}")
message(STATUS "===============================================")
