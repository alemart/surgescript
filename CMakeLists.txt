# -----------------------------------------------------------------------------
# SurgeScript
# A scripting language for games
# Copyright (C) 2018  Alexandre Martins <alemartf(at)gmail(dot)com>
# -----------------------------------------------------------------------------

# Project info
cmake_minimum_required(VERSION 3.0.2)
project(surgescript)

# Sources
file(GLOB_RECURSE SRC "src/*.c")

# Output folder
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Set build number
set(BUILD_FILE_PATH "${CMAKE_BINARY_DIR}/build_number.txt")
if(EXISTS ${BUILD_FILE_PATH})
    file(STRINGS ${BUILD_FILE_PATH} BUILD_NUMBER)
    add_definitions(-DSSBUILD="${BUILD_NUMBER}")
endif()

# Executable
add_executable(surgescript ${SRC})
target_link_libraries(surgescript m)