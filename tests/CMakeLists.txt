# Create an executable for running the tests
add_executable(runTests
    test_main.cpp
    test_sample.cpp  # Add more test files as you create them.
)

# Link the test executable against GoogleTest and your core library (and any other necessary libraries)
target_link_libraries(runTests
    PRIVATE gtest_main core
)

# Automatically discover and add tests to CTest
include(GoogleTest)
gtest_discover_tests(runTests)
