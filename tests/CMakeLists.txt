# tests/CMakeLists.txt

# Create the test executable
add_executable(runTests
    test_main.cpp
    test_tokenize.cpp
    test_dispatch.cpp
    test_config_parser.cpp
    test_file_io.cpp
    test_command_registry.cpp
    test_clear_command.cpp
    test_exit_command.cpp
    test_help_command.cpp
    test_ls_command.cpp
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

# Add coverage options for GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Compiler is GCC or Clang, adding coverage flags for runTests.")
    target_compile_options(runTests PRIVATE --coverage)
    target_link_libraries(runTests PRIVATE --coverage)
else()
    message(STATUS "Compiler is not GCC or Clang, coverage flags not added for runTests.")
endif()

# Discover and register tests with CTest
include(GoogleTest)
gtest_discover_tests(runTests
    PROPERTIES
    ENVIRONMENT "NEURODECK_PLUGIN_PATH=${CMAKE_SOURCE_DIR}/scripts/plugins"
)
