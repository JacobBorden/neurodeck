#include "gtest/gtest.h"
#include "../shell/commands/exit.hpp" // Include the header for ExitCommand
#include <vector>   // For std::vector
#include <string>   // For std::string
#include <memory>   // For std::unique_ptr

TEST(ExitCommandTest, NameIsCorrect) {
    std::unique_ptr<Command> cmd = make_exit();
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(cmd->name(), "exit");
}

TEST(ExitCommandTest, RunExecutesSuccessfully) {
    std::unique_ptr<Command> cmd = make_exit();
    ASSERT_NE(cmd, nullptr);
    
    std::vector<std::string> no_args = {};
    EXPECT_NO_THROW(cmd->run(no_args));
    
    std::vector<std::string> some_args = {"some", "args"};
    EXPECT_NO_THROW(cmd->run(some_args));
}
