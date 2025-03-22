cmake_minimum_required(VERSION 3.10)
project(deferred_printf)

set(CMAKE_CXX_STANDARD 17)

# Add the main library
add_library(deferred_printf
    include/deferred_printf.h
    src/deferred_printf.cpp
)

# Include directories
target_include_directories(deferred_printf PUBLIC include)

# Add the test executable
add_executable(test_deferred_printf tests/test_deferred_printf.cpp)

# Link the test executable with the main library
target_link_libraries(test_deferred_printf deferred_printf)

# Enable testing
enable_testing()

# Add a test
add_test(NAME DeferredPrintfTest COMMAND test_deferred_printf)