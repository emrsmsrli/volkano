#
# Copyright (C) 2022 Emre Simsirli
#
# Licensed under GPLv3 or any later version.
# Refer to the included LICENSE file.
#

cmake_minimum_required(VERSION 3.16)
project(external_libs)

function(add_header_only_library name)
    add_library(${name} INTERFACE)
    add_library(${name}::${name} ALIAS ${name})
    target_include_directories(${name} INTERFACE ${name}/)
    target_link_libraries(${name} INTERFACE project_options)
endfunction()

# add_header_only_library(cxxopts)
# add_header_only_library(access_private)
add_header_only_library(debugbreak)

add_subdirectory(imgui)
add_subdirectory(vma-hpp)