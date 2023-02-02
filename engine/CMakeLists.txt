#
# Copyright (C) 2022 Emre Simsirli
#
# Licensed under GPLv3 or any later version.
# Refer to the included LICENSE file.
#

cmake_minimum_required(VERSION 3.22)
project(libvolkano VERSION 0.0.1 DESCRIPTION "a vulkan based engine" LANGUAGES CXX)

configure_file(cmake/volkano_version.in
        include/version.h)

find_package(magic_enum CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

include(SetTargetStandard)

option(VKE_LOG_VERBOSITY "Log level: OFF, CRITICAL, ERROR, WARNING, INFO, DEBUG, VERBOSE" "WARNING")
set(LOG_VERBOSITY_LIST "OFF" "CRITICAL" "ERROR" "WARNING" "INFO" "DEBUG" "VERBOSE")
list(FIND LOG_VERBOSITY_LIST ${VKE_LOG_VERBOSITY} LOG_VERBOSITY_INDEX)

message(STATUS "volkano - Log level: ${VKE_LOG_VERBOSITY} (${LOG_VERBOSITY_INDEX})")
if(${LOG_VERBOSITY_INDEX} EQUAL -1)
    message(WARNING "Unknown log level, setting it to warning")
    list(FIND LOG_VERBOSITY_LIST "WARNING" LOG_VERBOSITY_INDEX)
endif()

add_library(${PROJECT_NAME}
        include/volkano.h
        include/version.h
        include/core/assert.h
        include/core/int_types.h
        include/core/platform.h
        include/core/type_traits.h
        include/core/algo/contains_if.h
        include/core/algo/find_ptr.h
        include/core/algo/index_of.h
        include/core/container/static_vector.h
        include/core/event/delegate.h
        include/core/filesystem/filesystem.h
        include/core/logging/logging.h
        include/core/logging/logging_types.h
        include/core/math/constants.h
        include/core/math/math_helpers.h
        include/core/math/vec2.h
        include/core/memory/aligned_union.h
        include/core/util/fmt_formatters.h
        include/renderer/null_renderer.h
        include/renderer/renderer_interface.h
        include/renderer/vk_include.h
        include/renderer/vk_renderer.h
        src/volkano.cpp
        src/core/filesystem/filesystem.cpp
        src/core/logging/logging.cpp
        src/renderer/vk_renderer.cpp
        src/renderer/vma_impl.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include/)
target_set_cxx_standard(${PROJECT_NAME} 20)
target_set_warnings(${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PUBLIC VKE_LOG_COMPILE_TIME_VERBOSITY=${LOG_VERBOSITY_INDEX})
target_precompile_headers(${PROJECT_NAME}
        PUBLIC include/volkano.h
        PRIVATE <algorithm> <concepts> <type_traits> <cstdint> <cstddef> <memory>
            <fmt/format.h>
            include/renderer/vk_include.h)
target_link_libraries(${PROJECT_NAME}
        PUBLIC fmt::fmt magic_enum::magic_enum
        PRIVATE Vulkan::Headers VulkanMemoryAllocatorHpp::Headers debugbreak::debugbreak SDL2::SDL2-static)

enable_sanitizers(${PROJECT_NAME})

if(MSVC)
    # Don't use the legacy preprocessor
    target_compile_options(${PROJECT_NAME} PUBLIC /Zc:preprocessor)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC DEBUG=$<IF:$<CONFIG:Debug>,1,0>)

add_library(volkano::engine ALIAS ${PROJECT_NAME})

set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders CACHE PATH "engine shader path")
set(SHADER_BINARIES)
set(SHADER_SOURCES
        ${SHADER_SRC_DIR}/triangle.vert
        ${SHADER_SRC_DIR}/triangle.frag)

foreach(SOURCE ${SHADER_SOURCES})
    get_filename_component(SOURCE_FILENAME ${SOURCE} NAME)
    set(BINARY ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SOURCE_FILENAME}.spr)
    add_custom_command(OUTPUT ${BINARY}
            COMMAND  ${glslc_executable} ${SOURCE} -o ${BINARY}
            DEPENDS ${SOURCE}
            COMMENT "Compiling ${SOURCE}")
    list(APPEND SHADER_BINARIES ${BINARY})
endforeach()

add_custom_target(shader_compile ALL DEPENDS ${SHADER_BINARIES})
add_dependencies(${PROJECT_NAME} shader_compile)
