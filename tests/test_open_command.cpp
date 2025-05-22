#include "gtest/gtest.h"
#include "../shell/commands/open.hpp" // Include the header for OpenCommand
#include <vector>   // For std::vector
#include <string>   // For std::string
#include <memory>   // For std::unique_ptr

TEST(OpenCommandTest, NameIsCorrect) {
    std::unique_ptr<Command> cmd = make_open();
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(cmd->name(), "open");
}

TEST(OpenCommandTest, RunExecutesSuccessfully) {
    std::unique_ptr<Command> cmd = make_open();
    ASSERT_NE(cmd, nullptr);
    
    EXPECT_NO_THROW(cmd->run({"test_file.txt"}));
    EXPECT_NO_THROW(cmd->run({}));
    EXPECT_NO_THROW(cmd->run({"file.txt", "another_arg"}));
}
