#
# Copyright (C) 2022 Emre Simsirli
#
# Licensed under GPLv3 or any later version.
# Refer to the included LICENSE file.
#

cmake_minimum_required(VERSION 3.22)
project(volkano_tests)

find_package(doctest CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
        engine/core/static_vector.cpp
        engine/core/string_utils.cpp
        main.cpp)

target_set_cxx_standard(${PROJECT_NAME} 20)
target_set_warnings(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE
        doctest::doctest
        volkano::engine)

add_test(NAME volkano_tests COMMAND ${PROJECT_NAME})
