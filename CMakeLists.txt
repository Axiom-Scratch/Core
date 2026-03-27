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

# Options
function(axiom_option NAME DEFAULT DESCRIPTION)
    option(${NAME} "${DESCRIPTION}" ${DEFAULT})
endfunction()

function(axiom_define_toggle TARGET NAME)
    if(${NAME})
        target_compile_definitions(${TARGET} PRIVATE ${NAME}=1)
    else()
        target_compile_definitions(${TARGET} PRIVATE ${NAME}=0)
    endif()
endfunction()

# Core toggles
axiom_option(AXIOM_ENABLE_ASSERTS ON "Enable engine assertions")
axiom_option(AXIOM_ENABLE_LOGGING ON "Enable engine logging")

# Sanitizers
axiom_option(AXIOM_ENABLE_ASAN ON "AddressSanitizer")
axiom_option(AXIOM_ENABLE_UBSAN ON "UBSan")
axiom_option(AXIOM_ENABLE_TSAN OFF "TSan")

# Tooling
axiom_option(AXIOM_ENABLE_TRACY ON "Tracy profiler")
axiom_option(AXIOM_ENABLE_LTO OFF "Link-time optimization")
axiom_option(AXIOM_WARNINGS_AS_ERRORS ON "Treat warnings as errors")

# Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler detection
if(MSVC)
    set(AXIOM_COMPILER MSVC)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(AXIOM_COMPILER GCC)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(AXIOM_COMPILER CLANG)
else()
    message(FATAL_ERROR "Unsupported compiler")
endif()

# Dependencies
message(STATUS "---- Dependencies ----")

add_subdirectory(external/glfw)

add_subdirectory(external/glad/cmake)
glad_add_library(glad REPRODUCIBLE API gl:core=4.6) # FIXED

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)

add_subdirectory(external/spdlog)

if(AXIOM_ENABLE_TRACY)
    add_subdirectory(external/tracy)
endif()

# Engine
add_subdirectory(Engine)

# TARGET CONFIGURATION

# Apply config to Engine target ONLY
function(axiom_configure_target TARGET)

    # Debug symbols
    if(AXIOM_COMPILER STREQUAL "MSVC")
        target_compile_options(${TARGET} PRIVATE /Zi)
    else()
        target_compile_options(${TARGET} PRIVATE -g)
    endif()

    # Frame pointer (perf tools)
    if(NOT AXIOM_COMPILER STREQUAL "MSVC")
        target_compile_options(${TARGET} PRIVATE -fno-omit-frame-pointer)
    endif()

    # Optimization per config
    target_compile_options(${TARGET} PRIVATE
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-O2>
    )

    # Warnings
    include(cmake/compiler_warnings.cmake)

    if(AXIOM_WARNINGS_AS_ERRORS)
        if(AXIOM_COMPILER STREQUAL "MSVC")
            target_compile_options(${TARGET} PRIVATE /WX)
        else()
            target_compile_options(${TARGET} PRIVATE -Werror)
        endif()
    endif()

    # Sanitizers
    if(AXIOM_ENABLE_ASAN OR AXIOM_ENABLE_UBSAN)
        if(AXIOM_COMPILER STREQUAL "MSVC")
            message(FATAL_ERROR "Sanitizers not supported on MSVC")
        endif()

        set(SAN_FLAGS "")

        if(AXIOM_ENABLE_ASAN)
            list(APPEND SAN_FLAGS address)
        endif()

        if(AXIOM_ENABLE_UBSAN)
            list(APPEND SAN_FLAGS undefined)
        endif()

        list(JOIN SAN_FLAGS "," SAN_FLAGS_STR)

        target_compile_options(${TARGET} PRIVATE -fsanitize=${SAN_FLAGS_STR})
        target_link_options(${TARGET} PRIVATE -fsanitize=${SAN_FLAGS_STR})
    endif()

    # LTO
    if(AXIOM_ENABLE_LTO)
        if(AXIOM_COMPILER STREQUAL "MSVC")
            target_compile_options(${TARGET} PRIVATE /GL)
            target_link_options(${TARGET} PRIVATE /LTCG)
        else()
            target_compile_options(${TARGET} PRIVATE -flto)
            target_link_options(${TARGET} PRIVATE -flto)
        endif()
    endif()

    # Feature toggles
    axiom_define_toggle(${TARGET} AXIOM_ENABLE_ASSERTS)
    axiom_define_toggle(${TARGET} AXIOM_ENABLE_LOGGING)

    # Tracy
    if(AXIOM_ENABLE_TRACY)
        target_compile_definitions(${TARGET} PRIVATE TRACY_ENABLE)
        target_sources(${TARGET} PRIVATE
            ${CMAKE_SOURCE_DIR}/external/tracy/public/TracyClient.cpp
        )
        target_link_libraries(${TARGET} PRIVATE Tracy::TracyClient)
    endif()

endfunction()

# Apply config to Engine
axiom_configure_target(Engine)

# compile_commands symlink
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_BINARY_DIR}/compile_commands.json
                ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()

# Info dump
message(STATUS "========== Axiom Build ==========")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C++: ${CMAKE_CXX_STANDARD}")
message(STATUS "ASAN: ${AXIOM_ENABLE_ASAN}")
message(STATUS "UBSAN: ${AXIOM_ENABLE_UBSAN}")
message(STATUS "TRACY: ${AXIOM_ENABLE_TRACY}")
message(STATUS "================================")
