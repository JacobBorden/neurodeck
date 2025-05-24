#include <gtest/gtest.h>
#include "command_registry.hpp" // Use CommandRegistry
#include "command.hpp"          // For Neurodeck::Command
#include <memory>
#include <vector>
#include <string>
#include <algorithm> // For std::find

// The old build_registry() is gone.
// Tests will now use Neurodeck::CommandRegistry and Neurodeck::populate_default_commands.

TEST(Dispatch, ContainsBuiltins) {
    Neurodeck::CommandRegistry registry;
    Neurodeck::populate_default_commands(registry);

    // Check for standard built-in commands
    EXPECT_NE(registry.get_command("ls"), nullptr);
    EXPECT_NE(registry.get_command("clear"), nullptr);
    EXPECT_NE(registry.get_command("help"), nullptr);
    EXPECT_NE(registry.get_command("open"), nullptr);
    EXPECT_NE(registry.get_command("exit"), nullptr);
    EXPECT_NE(registry.get_command("loadplugin"), nullptr);   // Added in previous subtask
    EXPECT_NE(registry.get_command("unloadplugin"), nullptr); // Added in previous subtask

    // Verify a non-existent command
    EXPECT_EQ(registry.get_command("nonexistentcommand"), nullptr);
}

// Example of how a more complex dispatch test might look (conceptual)
// This would require mocking or a way to capture output from commands.
// For now, this is just a placeholder to illustrate.
/*
TEST(Dispatch, LsCommandExecution) {
    Neurodeck::CommandRegistry registry;
    Neurodeck::populate_default_commands(registry);
    Neurodeck::Command* ls_cmd = registry.get_command("ls");
    ASSERT_NE(ls_cmd, nullptr);

    // To properly test execution, you'd need to:
    // 1. Capture std::cout (e.g., by redirecting its streambuf)
    // 2. Call ls_cmd->run({});
    // 3. Restore std::cout and check the captured output.
    // This is more involved and might be part of individual command tests.
    // For now, we'll just check if it runs without crashing.
    // This test is not robust as it doesn't check output.
    ASSERT_NO_THROW(ls_cmd->run({"ls"}));
}
*/
