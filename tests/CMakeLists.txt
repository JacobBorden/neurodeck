# tests/CMakeLists.txt

# Create the test executable
add_executable(runTests
    test_main.cpp
    test_tokenize.cpp
    test_dispatch.cpp
)

# Include directories for headers
target_include_directories(runTests PRIVATE
    ${CMAKE_SOURCE_DIR}/shell
    ${CMAKE_SOURCE_DIR}/core
)

# Link against GoogleTest, shell, and core libraries
target_link_libraries(runTests
    PRIVATE
    gtest_main
    shell
    core
)

# Discover and register tests with CTest
include(GoogleTest)
gtest_discover_tests(runTests)