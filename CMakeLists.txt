# ------------------------------------------------------------------------------
# SurgeScript
# A scripting language for games
# Copyright 2016-2020 Alexandre Martins <alemartf(at)gmail(dot)com>
# ------------------------------------------------------------------------------

# Project info
cmake_minimum_required(VERSION 3.2)
project(
    surgescript
    VERSION 0.5.4.4
    LANGUAGES C
)

# Default config
set(CMAKE_C_STANDARD 99)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build: Debug | Release | MinSizeRel | RelWithDebInfo" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
if(UNIX AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install path prefix (prepended onto install directories)" FORCE)
endif()

# User options
option(WANT_SHARED "Build SurgeScript as a shared library" ON)
option(WANT_STATIC "Build SurgeScript as a static library" ON)
option(WANT_EXECUTABLE "Build the surgescript executable" ON)
set(LIB_SUFFIX "" CACHE STRING "Suffix to append to 'lib' directories, e.g., '64'") # libs must be installed to "lib64" in some systems
set(PKGCONFIG_PATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig" CACHE PATH "Destination folder of the pkg-config (.pc) file")
if(UNIX)
    set(METAINFO_PATH "/usr/share/metainfo" CACHE PATH "Destination folder of the metainfo file")
    set(ICON_PATH "/usr/share/pixmaps" CACHE PATH "Destination folder of the icon file")
    file(TO_CMAKE_PATH "${ICON_PATH}/surgescript.png" ICON_FILEPATH)
endif()

# Output folder
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Sources
set(
    SURGESCRIPT_SOURCES
    src/surgescript/compiler/asm.c
    src/surgescript/compiler/lexer.c
    src/surgescript/compiler/parser.c
    src/surgescript/compiler/symtable.c
    src/surgescript/compiler/token.c
    src/surgescript/runtime/heap.c
    src/surgescript/runtime/object.c
    src/surgescript/runtime/object_manager.c
    src/surgescript/runtime/program.c
    src/surgescript/runtime/program_pool.c
    src/surgescript/runtime/renv.c
    src/surgescript/runtime/sslib/application.c
    src/surgescript/runtime/sslib/arguments.c
    src/surgescript/runtime/sslib/array.c
    src/surgescript/runtime/sslib/boolean.c
    src/surgescript/runtime/sslib/console.c
    src/surgescript/runtime/sslib/date.c
    src/surgescript/runtime/sslib/dictionary.c
    src/surgescript/runtime/sslib/gc.c
    src/surgescript/runtime/sslib/math.c
    src/surgescript/runtime/sslib/number.c
    src/surgescript/runtime/sslib/object.c
    src/surgescript/runtime/sslib/plugin.c
    src/surgescript/runtime/sslib/string.c
    src/surgescript/runtime/sslib/surgescript.c
    src/surgescript/runtime/sslib/system.c
    src/surgescript/runtime/sslib/tags.c
    src/surgescript/runtime/sslib/temp.c
    src/surgescript/runtime/sslib/time.c
    src/surgescript/runtime/stack.c
    src/surgescript/runtime/tag_system.c
    src/surgescript/runtime/variable.c
    src/surgescript/runtime/vm.c
    src/surgescript/util/transform.c
    src/surgescript/util/utf8.c
    src/surgescript/util/util.c
    src/surgescript/util/xoroshiro128plus.c
    ${CMAKE_CURRENT_BINARY_DIR}/info.c
)

# Headers
set(
    SURGESCRIPT_HEADERS
    src/surgescript/compiler/asm.h
    src/surgescript/compiler/lexer.h
    src/surgescript/compiler/nodecontext.h
    src/surgescript/compiler/parser.h
    src/surgescript/compiler/symtable.h
    src/surgescript/compiler/token.h
    src/surgescript/runtime/heap.h
    src/surgescript/runtime/object.h
    src/surgescript/runtime/object_manager.h
    src/surgescript/runtime/program.h
    src/surgescript/runtime/program_operators.h
    src/surgescript/runtime/program_pool.h
    src/surgescript/runtime/renv.h
    src/surgescript/runtime/sslib/sslib.h
    src/surgescript/runtime/stack.h
    src/surgescript/runtime/tag_system.h
    src/surgescript/runtime/variable.h
    src/surgescript/runtime/vm.h
    src/surgescript/util/fasthash.h
    src/surgescript/util/ssarray.h
    src/surgescript/util/transform.h
    src/surgescript/util/utf8.h
    src/surgescript/util/uthash.h
    src/surgescript/util/util.h
    src/surgescript.h
)

# Generate files from templates
function(generate_file TEMPLATE)
    configure_file(src/surgescript/misc/${TEMPLATE}.in ${TEMPLATE} @ONLY)
endfunction()

function(generate_pc_file LINKAGE)
    set(LIB_LINKAGE "")
    if(LINKAGE STREQUAL "static")
        set(LIB_LINKAGE "-static")
    endif()
    configure_file(src/surgescript/misc/surgescript.pc.in "${LIBRARY_OUTPUT_PATH}/surgescript${LIB_LINKAGE}.pc" @ONLY)
    install(FILES "${LIBRARY_OUTPUT_PATH}/surgescript${LIB_LINKAGE}.pc" DESTINATION "${PKGCONFIG_PATH}")
endfunction()

generate_file("info.c")


# Library
if(NOT WANT_SHARED AND NOT WANT_STATIC)
    message(FATAL_ERROR "Options WANT_SHARED and WANT_STATIC are both set to OFF. Nothing to do.")
endif()

if(WANT_SHARED)
    message(STATUS "Will build libsurgescript")
    generate_pc_file("shared")
    add_library(surgescript SHARED ${SURGESCRIPT_SOURCES} ${SURGESCRIPT_HEADERS})
    target_link_libraries(surgescript m)
    set_target_properties(surgescript PROPERTIES VERSION ${PROJECT_VERSION})
    install(TARGETS surgescript DESTINATION "lib${LIB_SUFFIX}")
endif()

if(WANT_STATIC)
    message(STATUS "Will build libsurgescript-static")
    generate_pc_file("static")
    add_library(surgescript-static STATIC ${SURGESCRIPT_SOURCES} ${SURGESCRIPT_HEADERS})
    target_link_libraries(surgescript-static m)
    set_target_properties(surgescript-static PROPERTIES VERSION ${PROJECT_VERSION})
    install(TARGETS surgescript-static DESTINATION "lib${LIB_SUFFIX}")
endif()

install(DIRECTORY src/ DESTINATION include FILES_MATCHING PATTERN "*.h") # installing the headers

# Executable
if(WANT_EXECUTABLE)
    # Set the appropriate lib
    set(LIBSURGESCRIPT "surgescript")
    if(WANT_STATIC AND (NOT WANT_SHARED OR WIN32))
        set(LIBSURGESCRIPT "surgescript-static")
    endif()

    # Create the executable
    message(STATUS "Will build surgescript")
    add_executable(surgescript.bin src/main.c)
    target_link_libraries(surgescript.bin ${LIBSURGESCRIPT})
    target_include_directories(surgescript.bin PRIVATE src)
    set_target_properties(surgescript.bin PROPERTIES OUTPUT_NAME surgescript)

    # Windows icon
    if(WIN32 AND MINGW)
        if(NOT CMAKE_RC_COMPILER)
            set(CMAKE_RC_COMPILER windres)
        endif()
        execute_process(COMMAND "${CMAKE_RC_COMPILER}" -O coff -o "${CMAKE_CURRENT_BINARY_DIR}/iconwin.res" -i "${CMAKE_SOURCE_DIR}/src/surgescript/misc/iconwin.rc" -I "${CMAKE_SOURCE_DIR}")
        set_target_properties(surgescript.bin PROPERTIES LINK_FLAGS "-static-libgcc \"${CMAKE_CURRENT_BINARY_DIR}/iconwin.res\"")
    endif()

    # *nix metadata & icon
    if(UNIX)
        generate_file("surgescript.appdata.xml")
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/surgescript.appdata.xml DESTINATION "${METAINFO_PATH}")
        install(FILES src/surgescript/misc/surgescript.png DESTINATION "${ICON_PATH}")
    endif()

    # Installing the executable
    install(TARGETS surgescript.bin DESTINATION bin)
endif()
