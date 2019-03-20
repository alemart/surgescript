# ------------------------------------------------------------------------------
# SurgeScript
# A scripting language for games
# Copyright 2016-2019 Alexandre Martins <alemartf(at)gmail(dot)com>
# ------------------------------------------------------------------------------

# Project info
cmake_minimum_required(VERSION 3.0.2)
project(surgescript)

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
    src/surgescript/util/lookup3.c
    src/surgescript/util/transform.c
    src/surgescript/util/utf8.c
    src/surgescript/util/util.c
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
    src/surgescript/util/ssarray.h
    src/surgescript/util/transform.h
    src/surgescript/util/utf8.h
    src/surgescript/util/uthash.h
    src/surgescript/util/util.h
    src/surgescript.h
)

# Output folder
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Library
add_library(surgescript STATIC ${SURGESCRIPT_SOURCES} ${SURGESCRIPT_HEADERS})
target_link_libraries(surgescript m)

# Executable
add_executable(surgescript.bin src/main.c)
target_link_libraries(surgescript.bin surgescript)
target_include_directories(surgescript.bin PRIVATE src)
set_target_properties(surgescript.bin PROPERTIES OUTPUT_NAME surgescript)

# Windows icon
if(WIN32 AND MINGW)
    if(NOT CMAKE_RC_COMPILER)
        set(CMAKE_RC_COMPILER windres)
    endif()
    execute_process(COMMAND "${CMAKE_RC_COMPILER}" -O coff -o "${CMAKE_CURRENT_BINARY_DIR}/iconwin.res" -i "${CMAKE_SOURCE_DIR}/src/surgescript/resources/iconwin.rc" -I "${CMAKE_SOURCE_DIR}")
    set_target_properties(surgescript.bin PROPERTIES LINK_FLAGS "\"${CMAKE_CURRENT_BINARY_DIR}/iconwin.res\"")
endif()

# Installation rules
install(DIRECTORY src/ DESTINATION include FILES_MATCHING PATTERN "*.h")
install(TARGETS surgescript DESTINATION lib)
install(TARGETS surgescript.bin DESTINATION bin)