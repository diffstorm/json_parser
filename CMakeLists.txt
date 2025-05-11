# Copyright (c) 2025 Eray Ozturk <erayozturk1@gmail.com>
# url github.com/diffstorm
cmake_minimum_required(VERSION 3.12)
project(json_parser LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library configuration
add_library(json_parser STATIC
    json_parser.h
    json_parser.c
)

# Increase token count for validation test
target_compile_definitions(json_parser PRIVATE JSON_DEFAULT_MAX_TOKENS=5120)

set_target_properties(json_parser PROPERTIES
    C_VISIBILITY_PRESET hidden
    INTERPROCEDURAL_OPTIMIZATION FALSE
)

target_include_directories(json_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# C demo
add_executable(demo
    demo.c
)

# C++17 demo
add_executable(demo_cpp
    demo.cpp
)

# Link demo to library
target_link_libraries(demo PRIVATE json_parser)
target_link_libraries(demo_cpp PRIVATE json_parser)

# Google Test configuration
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Test executable
add_executable(json_parser_tests
    json_parser_test.cpp
)

target_link_libraries(json_parser_tests
    PRIVATE
    json_parser
    GTest::GTest
    GTest::Main
    Threads::Threads
)

# Validation Test executable
add_executable(validation_test
    validation_test.cpp
)

target_link_libraries(validation_test
    PRIVATE
    json_parser
    GTest::GTest
    GTest::Main
    Threads::Threads
)

enable_testing()
add_test(NAME json_parser_tests COMMAND json_parser_tests)
add_test(NAME validation_test COMMAND validation_test)

# Installation configuration
install(TARGETS json_parser
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION include
)

install(FILES json_parser.h
    DESTINATION include
)
