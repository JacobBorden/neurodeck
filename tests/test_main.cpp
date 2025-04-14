#include <gtest/gtest.h>

// Main entry point for all GoogleTest tests.
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
