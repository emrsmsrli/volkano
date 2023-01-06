#
# Copyright (C) 2022 Emre Simsirli
#
# Licensed under GPLv3 or any later version.
# Refer to the included LICENSE file.
#

cmake_minimum_required(VERSION 3.22)
project(volkano VERSION 0.0.1 DESCRIPTION "a vulkan based engine" LANGUAGES CXX)

configure_file(cmake/volkano_version.in
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/version.h)

find_package(magic_enum CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

include(SetTargetStandard)

option(VKE_LOG_VERBOSITY "Log level: OFF, CRITICAL, ERROR, WARNING, INFO, DEBUG, VERBOSE" "WARNING")
set(LOG_VERBOSITY_LIST "OFF" "CRITICAL" "ERROR" "WARNING" "INFO" "DEBUG" "VERBOSE")
list(FIND LOG_VERBOSITY_LIST ${VKE_LOG_VERBOSITY} LOG_VERBOSITY_INDEX)

message(STATUS "Log level: ${VKE_LOG_VERBOSITY} (${LOG_VERBOSITY_INDEX})")
if(${LOG_VERBOSITY_INDEX} EQUAL -1)
    message(WARNING "Unknown log level, setting it to warning")
    list(FIND LOG_VERBOSITY_LIST "WARNING" LOG_VERBOSITY_INDEX)
endif()

add_library(${PROJECT_NAME}
        include/engine/volkano.h
        include/engine/version.h
        include/engine/core/assert.h
        include/engine/core/int_types.h
        include/engine/core/platform.h
        include/engine/core/type_traits.h
        include/engine/core/algo/contains.h
        include/engine/core/algo/find.h
        include/engine/core/algo/index_of.h
        include/engine/core/container/static_vector.h
        include/engine/core/event/delegate.h
        include/engine/core/filesystem/filesystem.h
        include/engine/core/logging/logging.h
        include/engine/core/logging/logging_types.h
        include/engine/core/math/constants.h
        include/engine/core/math/math_helpers.h
        include/engine/core/memory/aligned_union.h
        include/engine/core/util/fmt_formatters.h
        src/core/filesystem/filesystem.cpp
        src/core/logging/logging.cpp)

target_include_directories(${PROJECT_NAME}
        PRIVATE src/
        PUBLIC include/)
target_set_cxx_standard(${PROJECT_NAME} 20)
target_set_warnings(${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PUBLIC VKE_LOG_COMPILE_TIME_VERBOSITY=${LOG_VERBOSITY_INDEX})
target_precompile_headers(${PROJECT_NAME}
        PUBLIC include/engine/volkano.h
        PRIVATE <algorithm> <concepts> <type_traits> <cstdint> <cstddef> <memory> <ranges> )
target_link_libraries(${PROJECT_NAME}
        PUBLIC fmt::fmt magic_enum::magic_enum
        PRIVATE Vulkan::Vulkan debugbreak::debugbreak)

enable_sanitizers(${PROJECT_NAME})

if(MSVC)
    # Don't use the legacy preprocessor
    target_compile_options(${PROJECT_NAME} PUBLIC /Zc:preprocessor)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
            PLATFORM_WIN32=1
            PLATFORM_UNIX=0)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC
            PLATFORM_WIN32=0
            PLATFORM_UNIX=1)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC DEBUG=$<IF:$<CONFIG:Debug>,1,0>)

add_library(volkano::engine ALIAS ${PROJECT_NAME})
