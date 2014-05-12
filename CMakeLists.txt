cmake_minimum_required (VERSION 2.6)
project (dict)

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_PATCH 0)
set (CMAKE_C_FLAGS "-Wall -g -std=c99 -pedantic")

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/config.h"
)

include_directories(
    "${PROJECT_SOURCE_DIR}"
)

set(LIB_SOURCES
    crc32.c
    dict.c
)

add_library(dict ${LIB_SOURCES})

add_executable(tests/bin/basic-test tests/basic-test.c)
target_link_libraries(tests/bin/basic-test dict)
