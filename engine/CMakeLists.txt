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

option(VK_LOG_VERBOSITY "Log level: OFF, CRITICAL, ERROR, WARNING, INFO, DEBUG, VERBOSE" "WARNING")
set(LOG_VERBOSITY_LIST "OFF" "CRITICAL" "ERROR" "WARNING" "INFO" "DEBUG" "VERBOSE")
list(FIND LOG_VERBOSITY_LIST ${VK_LOG_VERBOSITY} LOG_VERBOSITY_INDEX)

message(STATUS "Log level: ${VK_LOG_VERBOSITY} (${LOG_VERBOSITY_INDEX})")
if(${LOG_VERBOSITY_INDEX} EQUAL -1)
    message(WARNING "Unknown log level, setting it to warning")
    list(FIND LOG_VERBOSITY_LIST "WARNING" LOG_VERBOSITY_INDEX)
endif()

add_library(${PROJECT_NAME} STATIC
        src/string/string_utils.cpp
        src/logging/logging.cpp)

target_include_directories(${PROJECT_NAME}
        PRIVATE src/
        PUBLIC include/)
target_set_cxx_standard(${PROJECT_NAME} 20)
target_compile_definitions(${PROJECT_NAME} PUBLIC VK_LOG_COMPILE_TIME_VERBOSITY=${LOG_VERBOSITY_INDEX})
target_precompile_headers(${PROJECT_NAME}
        PRIVATE include/engine/volkano.h)
target_link_libraries(${PROJECT_NAME}
        PRIVATE Vulkan::Vulkan
        PUBLIC
            fmt::fmt
            range-v3-concepts
            magic_enum::magic_enum)

if(MSVC)
    # Don't use the legacy preprocessor
    target_compile_options(${PROJECT_NAME} PUBLIC /Zc:preprocessor)
endif()

target_set_warnings(${PROJECT_NAME})
enable_sanitizers(${PROJECT_NAME})

add_library(volkano::engine ALIAS ${PROJECT_NAME})
