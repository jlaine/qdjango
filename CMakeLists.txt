cmake_minimum_required(VERSION 2.6)

# Project and version
project(QDjango)
set(QDJANGO_VERSION 0.2.0)

find_package(Qt4 REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/db)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/http)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/script)

add_subdirectory(src)
if(NOT QDJANGO_DONT_USE_TESTS)
    add_subdirectory(tests)
    add_subdirectory(examples)
endif()
