# tests/CMakeLists.txt

# Create the test executable
add_executable(runTests
    test_main.cpp
    test_tokenize.cpp
    test_dispatch.cpp
    test_config_parser.cpp
    test_file_io.cpp
    test_command_registry.cpp
    test_exit_command.cpp
    test_help_command.cpp
    test_open_command.cpp
    test_exec_command.cpp
    test_load_plugin_command.cpp
    test_unload_plugin_command.cpp
    test_lua_command.cpp
    test_unified_plugin_system.cpp
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

target_compile_definitions(runTests PRIVATE TEST_SCRIPT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_scripts")

add_executable(neurodeck_mixed_pipe_acceptance_probe
    ../characterization/mixed-pipe-acceptance-v1/probe.cpp
)
target_link_libraries(neurodeck_mixed_pipe_acceptance_probe PRIVATE shell)

# Add coverage options for GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Compiler is GCC or Clang, adding coverage flags for runTests.")
    target_compile_options(runTests PRIVATE --coverage)
    target_link_libraries(runTests PRIVATE --coverage)
    target_compile_options(neurodeck_mixed_pipe_acceptance_probe PRIVATE --coverage)
    target_link_libraries(neurodeck_mixed_pipe_acceptance_probe PRIVATE --coverage)
else()
    message(STATUS "Compiler is not GCC or Clang, coverage flags not added for runTests.")
endif()

# Native plugins resolve CommandRegistry symbols from their test host.
set_target_properties(runTests PROPERTIES ENABLE_EXPORTS ON)
add_dependencies(runTests hello_plugin echo_plugin)

# Use build-resolved fixture paths across CTest discovery versions.
target_compile_definitions(runTests PRIVATE
    "DEFAULT_LUA_PLUGIN_DIR=\"${CMAKE_SOURCE_DIR}/scripts/plugins\""
    "DEFAULT_CPP_HELLO_PLUGIN_PATH=\"$<TARGET_FILE:hello_plugin>\""
    "DEFAULT_CPP_ECHO_PLUGIN_PATH=\"$<TARGET_FILE:echo_plugin>\""
)

# Discover and register tests with CTest
include(GoogleTest)
gtest_discover_tests(runTests PROPERTIES TIMEOUT 10)

# Keep syscall fault injection in a separate executable; the shell and other
# test binaries continue to call libc directly.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_executable(neurodeck_exec_launch_tests test_exec_launch.cpp)
    target_link_libraries(neurodeck_exec_launch_tests PRIVATE gtest_main shell)
    set_target_properties(neurodeck_exec_launch_tests PROPERTIES
        LINK_FLAGS "-Wl,--wrap=pipe,--wrap=fork,--wrap=waitpid,--wrap=poll")
    gtest_discover_tests(neurodeck_exec_launch_tests PROPERTIES TIMEOUT 10)
endif()

# Test the actual shell loop rather than the dispatch simulation in unit tests.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME ShellSession COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/test_shell_session.py $<TARGET_FILE:neurodeck_shell>)
set_tests_properties(ShellSession PROPERTIES TIMEOUT 40)
