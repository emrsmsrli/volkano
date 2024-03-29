#
# Copyright (C) 2022 Emre Simsirli
#
# Licensed under GPLv3 or any later version.
# Refer to the included LICENSE file.
#

cmake_minimum_required(VERSION 3.22)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_policy(SET CMP0091 NEW)  # See CMAKE_MSVC_RUNTIME_LIBRARY
project(volkano)

# todo extend CMAKE_BUILD_TYPE (release debug etc) where lto is enabled for some targets
# see https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#build-configurations

include(SetStandardProjectSettings)
include(SetProjectWarnings)
include(EnableSanitizers)
include(EnableDoxygen)
include(CheckStaticAnalyzers)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

option(VKE_ENABLE_TESTS "Enable Tests" OFF)
option(VKE_ENABLE_ASSERTIONS "Enable assertions" OFF)

add_library(project_options INTERFACE)
target_compile_definitions(project_options INTERFACE
        VKE_DEBUG=$<CONFIG:Debug>
        $<$<BOOL:${VKE_ENABLE_ASSERTIONS}>:VKE_ENABLE_ASSERTIONS>)

find_package(fmt CONFIG REQUIRED)

add_subdirectory(external/)
add_subdirectory(engine/)

if(VKE_ENABLE_TESTS)
    message(STATUS "volkano - Enabling tests")
    enable_testing()
    add_subdirectory(test/)
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE volkano::engine)
