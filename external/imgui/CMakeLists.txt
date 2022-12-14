#
# Copyright (C) 2022 Emre Simsirli
#
# Licensed under GPLv3 or any later version.
# Refer to the included LICENSE file.
#

cmake_minimum_required(VERSION 3.21)
project(imguicpp)

include(SetTargetStandard)

find_package(fmt CONFIG REQUIRED)

add_library(${PROJECT_NAME} STATIC
        include/imgui_cpp.h
        include/imconfig.h
        include/imgui.h
        include/imgui_internal.h
        include/imstb_rectpack.h
        include/imstb_textedit.h
        include/imstb_truetype.h
        src/imgui.cpp
        src/imgui_draw.cpp
        src/imgui_widgets.cpp
        src/imgui_tables.cpp
        src/imgui_demo.cpp)

add_library(imgui::cpp ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} fmt::fmt-header-only)
target_include_directories(${PROJECT_NAME}
        PUBLIC include/)
target_set_cxx_standard(${PROJECT_NAME} 20)
