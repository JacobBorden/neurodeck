#include <gtest/gtest.h>
#include "tokenize.hpp"

TEST(Tokenize, SplitsOnWhitespace) {
    auto v = tokenize("open ide --force");
    ASSERT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], "open");
    EXPECT_EQ(v[1], "ide");
    EXPECT_EQ(v[2], "--force");
}
