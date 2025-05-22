#include <gtest/gtest.h>
#include "tokenize.hpp"

TEST(Tokenize, SplitsOnWhitespace) {
    auto v = tokenize("open ide --force");
    ASSERT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], "open");
    EXPECT_EQ(v[1], "ide");
    EXPECT_EQ(v[2], "--force");
}

TEST(Tokenize, TestTokenizeEmptyInput) {
    auto v = tokenize("");
    ASSERT_EQ(v.size(), 0);
}

TEST(Tokenize, TestTokenizeWhitespaceOnlyInput) {
    auto v1 = tokenize("   ");
    ASSERT_EQ(v1.size(), 0);

    auto v2 = tokenize("\t \n "); // Mixed whitespace
    ASSERT_EQ(v2.size(), 0);
}

TEST(Tokenize, TestTokenizeLeadingAndTrailingWhitespace) {
    auto v = tokenize("  command arg  ");
    ASSERT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], "command");
    EXPECT_EQ(v[1], "arg");
}

TEST(Tokenize, TestTokenizeMultipleSpacesBetweenTokens) {
    auto v = tokenize("command   arg1   arg2");
    ASSERT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], "command");
    EXPECT_EQ(v[1], "arg1");
    EXPECT_EQ(v[2], "arg2");
}

TEST(Tokenize, TestTokenizeMixedWhitespaceBetweenTokens) {
    auto v = tokenize("command\targ1\narg2");
    ASSERT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], "command");
    EXPECT_EQ(v[1], "arg1");
    EXPECT_EQ(v[2], "arg2");
}

TEST(Tokenize, TestTokenizeSingleToken) {
    auto v = tokenize("command");
    ASSERT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], "command");
}

TEST(Tokenize, TestTokenizeSingleTokenWithWhitespace) {
    auto v = tokenize("  command  ");
    ASSERT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], "command");
}
