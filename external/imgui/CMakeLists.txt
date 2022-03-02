cmake_minimum_required(VERSION 3.21)
project(imguicpp)

include(SetTargetStandard)

find_package(fmt CONFIG REQUIRED)

add_library(${PROJECT_NAME} STATIC
        include/imgui_cpp.h
        src/imconfig.h
        src/imgui.h
        src/imgui_internal.h
        src/imstb_rectpack.h
        src/imstb_textedit.h
        src/imstb_truetype.h
        src/imgui.cpp
        src/imgui_draw.cpp
        src/imgui_widgets.cpp
        src/imgui_tables.cpp
        src/imgui_demo.cpp)

add_library(imgui::cpp ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} fmt::fmt-header-only)
target_include_directories(${PROJECT_NAME}
        PRIVATE src/
        PUBLIC include/)
target_set_cxx_standard(${PROJECT_NAME} 20)
