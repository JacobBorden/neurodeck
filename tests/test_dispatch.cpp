#include <gtest/gtest.h>
#include "command.hpp"
#include <memory>
#include <sstream>

extern std::unordered_map<std::string, std::unique_ptr<Command>> build_registry();

TEST(Dispatch, ContainsBuiltins) {
    auto reg = build_registry();
    EXPECT_TRUE(reg.find("ls")    != reg.end());
    EXPECT_TRUE(reg.find("clear") != reg.end());
    EXPECT_TRUE(reg.find("help")  != reg.end());
    EXPECT_TRUE(reg.find("open") != reg.end());
    EXPECT_TRUE(reg.find("exit") != reg.end());
}
