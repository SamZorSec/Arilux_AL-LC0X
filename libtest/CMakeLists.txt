cmake_minimum_required(VERSION 3.0)

project(cmake_test)

set(LIB "../lib/hsb")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

set(LIB_SOURCES
    ../lib/hsb/hsb.cpp
    ../lib/optparser/optparser.cpp
    ../lib/settings/settings.cpp
)
set(LIB_HEADERS
    ../lib/hsb
    ../lib/optparser
    ../lib/settings
)

include_directories(catch2 ${LIB_HEADERS})

# Make test executable
add_executable(tests main.cpp ${LIB_SOURCES})
target_link_libraries(tests Catch)