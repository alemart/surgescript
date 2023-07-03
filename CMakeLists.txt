# ------------------------------------------------------------------------------
# SurgeScript
# A scripting language for games
# Copyright 2016-2023 Alexandre Martins <alemartf(at)gmail(dot)com>
# ------------------------------------------------------------------------------

# Project info
cmake_minimum_required(VERSION 3.2)
project(
    surgescript
    VERSION 0.6.0
    LANGUAGES C
)
include(GNUInstallDirs)
include(CheckLibraryExists)
set(PROJECT_YEARS "2016-2023")

# Default config
set(CMAKE_C_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build: Debug | Release | MinSizeRel | RelWithDebInfo" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# User options
option(WANT_SHARED "Build SurgeScript as a shared library" ON)
option(WANT_STATIC "Build SurgeScript as a static library" ON)
option(WANT_EXECUTABLE "Build the SurgeScript CLI" ON)
option(WANT_EXECUTABLE_MULTITHREAD "Enable multithreading on the SurgeScript CLI" ON)
set(PKGCONFIG_PATH "pkgconfig" CACHE PATH "Destination folder of the pkg-config (.pc) file")
if(UNIX)
    set(METAINFO_PATH "metainfo" CACHE PATH "Destination folder of the metainfo file")
    set(ICON_PATH "pixmaps" CACHE PATH "Destination folder of the icon file")
endif()

# Library search
CHECK_LIBRARY_EXISTS(m sqrt "${CMAKE_SYSTEM_LIBRARY_PATH}" SURGESCRIPT_libm_EXISTS)

# Sources
set(
    SURGESCRIPT_SOURCES
    src/surgescript/compiler/asm.c
    src/surgescript/compiler/lexer.c
    src/surgescript/compiler/parser.c
    src/surgescript/compiler/symtable.c
    src/surgescript/compiler/token.c
    src/surgescript/runtime/heap.c
    src/surgescript/runtime/managed_string.c
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
    src/surgescript/runtime/vm_time.c
    src/surgescript/third_party/utf8.c
    src/surgescript/third_party/xoroshiro128plus.c
    src/surgescript/util/transform.c
    src/surgescript/util/util.c
    ${CMAKE_BINARY_DIR}/src/surgescript/misc/info.c
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
    src/surgescript/runtime/managed_string.h
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
    src/surgescript/runtime/vm_time.h
    src/surgescript/third_party/gettimeofday.h
    src/surgescript/third_party/utf8.h
    src/surgescript/third_party/uthash.h
    src/surgescript/third_party/xxh3.h
    src/surgescript/third_party/xxhash.h
    src/surgescript/util/fasthash.h
    src/surgescript/util/ssarray.h
    src/surgescript/util/transform.h
    src/surgescript/util/util.h
    src/surgescript.h
)

# Output folder
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Generate files from templates
function(generate_file TEMPLATE)
    configure_file(src/surgescript/${TEMPLATE}.in "${CMAKE_BINARY_DIR}/src/surgescript/${TEMPLATE}" @ONLY)
endfunction()

function(generate_pc_file LINKAGE)
    set(LIB_LINKAGE "")
    if(LINKAGE STREQUAL "static")
        set(LIB_LINKAGE "-static")
    endif()
    configure_file(src/surgescript/misc/surgescript.pc.in "${LIBRARY_OUTPUT_PATH}/surgescript${LIB_LINKAGE}.pc" @ONLY)
    install(FILES "${LIBRARY_OUTPUT_PATH}/surgescript${LIB_LINKAGE}.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PKGCONFIG_PATH}")
endfunction()

generate_file("misc/info.c")
generate_file("util/version.h")

# Use relative paths in __FILE__
function(drop_compilation_paths TARGET)
    if(NOT MSVC)
        target_compile_options(${TARGET} PUBLIC "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.")
    endif()
endfunction()

# Build the library
if(NOT WANT_SHARED AND NOT WANT_STATIC)
    message(FATAL_ERROR "Options WANT_SHARED and WANT_STATIC are both set to OFF. Nothing to do.")
endif()

if(WANT_SHARED)
    set(LIB_SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}") # x.y.z: backwards compatibility
    message(STATUS "Will build libsurgescript")
    generate_pc_file("shared")
    add_library(surgescript SHARED ${SURGESCRIPT_SOURCES} ${SURGESCRIPT_HEADERS})
    if (SURGESCRIPT_libm_EXISTS)
        target_link_libraries(surgescript m)
    endif()
    set_target_properties(surgescript PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${LIB_SOVERSION})
    drop_compilation_paths(surgescript)
endif()

if(WANT_STATIC)
    message(STATUS "Will build libsurgescript-static")
    generate_pc_file("static")
    add_library(surgescript-static STATIC ${SURGESCRIPT_SOURCES} ${SURGESCRIPT_HEADERS})
    if (SURGESCRIPT_libm_EXISTS)
        target_link_libraries(surgescript-static m)
    endif ()
    set_target_properties(surgescript-static PROPERTIES VERSION ${PROJECT_VERSION})
    drop_compilation_paths(surgescript-static)
endif()

# Install the headers
install(DIRECTORY src/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "${CMAKE_BINARY_DIR}/src/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" FILES_MATCHING PATTERN "*.h")

# Install the library
if(WANT_SHARED)
    install(TARGETS surgescript DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

if(WANT_STATIC)
    install(TARGETS surgescript-static DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

# Build the SurgeScript CLI
if(WANT_EXECUTABLE)
    message(STATUS "Will build the SurgeScript CLI")

    # Set the appropriate lib
    set(LIBSURGESCRIPT "surgescript")
    if(WANT_STATIC AND (NOT WANT_SHARED OR WIN32))
        set(LIBSURGESCRIPT "surgescript-static")
    endif()

    # Use multithreading?
    set(LIBTHREAD "")
    set(ENABLE_THREADS 0)
    if(WANT_EXECUTABLE_MULTITHREAD)

        # Header search
        find_path(THREADS_H NAMES "threads.h" PATHS "${CMAKE_INCLUDE_PATH}")
        if(NOT THREADS_H)
            message(WARNING "Can't find threads.h. Will not use multithreading on the SurgeScript CLI")
        else()
            message(STATUS "Will use multithreading on the SurgeScript CLI")
            set(ENABLE_THREADS 1)

            # hmmmm...
            set(LIBTHREAD "pthread")
        endif()

    endif()

    # Create the executable
    add_executable(surgescript.bin src/main.c)
    include_directories("${CMAKE_BINARY_DIR}/src")
    target_compile_definitions(surgescript.bin PUBLIC ENABLE_THREADS=${ENABLE_THREADS})
    target_link_libraries(surgescript.bin ${LIBSURGESCRIPT} ${LIBTHREAD})
    target_include_directories(surgescript.bin PRIVATE src)
    set_target_properties(surgescript.bin PROPERTIES OUTPUT_NAME surgescript)
    drop_compilation_paths(surgescript.bin)

    # WebAssembly
    if(EMSCRIPTEN)
        target_link_options(surgescript.bin PRIVATE -Os --closure 1 -sMODULARIZE=1 -sMALLOC="emmalloc" -sEXPORTED_FUNCTIONS=["_main"]) # Reduce output size
    endif()

    # Windows icon
    if(WIN32 AND MINGW)
        if(NOT CMAKE_RC_COMPILER)
            set(CMAKE_RC_COMPILER windres)
        endif()
        execute_process(COMMAND "${CMAKE_RC_COMPILER}" -O coff -o "${CMAKE_BINARY_DIR}/iconwin.res" -i "${CMAKE_SOURCE_DIR}/src/surgescript/misc/iconwin.rc" -I "${CMAKE_SOURCE_DIR}")
        set_target_properties(surgescript.bin PROPERTIES LINK_FLAGS "-static-libgcc \"${CMAKE_BINARY_DIR}/iconwin.res\"")
    endif()

    # *nix metadata & icon
    if(UNIX)
        file(TO_CMAKE_PATH "${CMAKE_INSTALL_DATADIR}/${ICON_PATH}/surgescript.png" ICON_FILEPATH)
        generate_file("misc/surgescript.appdata.xml")
        install(FILES "${CMAKE_BINARY_DIR}/src/surgescript/misc/surgescript.appdata.xml" DESTINATION "${CMAKE_INSTALL_DATADIR}/${METAINFO_PATH}")
        install(FILES src/surgescript/misc/surgescript.png DESTINATION "${CMAKE_INSTALL_DATADIR}/${ICON_PATH}")
    endif()

    # Installing the executable
    install(TARGETS surgescript.bin DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()
