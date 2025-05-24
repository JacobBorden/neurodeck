cmake_minimum_required(VERSION 3.10)
project(NeurodeckPlugins CXX)

# Set C++ standard (optional, but good practice)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Configuration for finding project headers ---
# This assumes that this CMakeLists.txt is in a 'plugins' subdirectory
# and the main project root contains 'shell' and 'core' directories.
# The main CMakeLists.txt (in the root) should ideally add the 'shell'
# and 'core' include directories to the INTERFACE of their respective targets,
# or globally. For now, we add them directly for the plugins.

# Get the directory of the current CMakeLists.txt (plugins/)
get_filename_component(PLUGINS_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
# Get the parent directory (project root)
get_filename_component(PROJECT_ROOT ${PLUGINS_DIR} DIRECTORY)

include_directories(
    "${PROJECT_ROOT}/shell" # For command.hpp, command_registry.hpp
    "${PROJECT_ROOT}/core"  # For any core utilities if needed in future plugins
)

# --- Hello Plugin ---
add_library(hello_plugin MODULE hello_plugin.cpp)
set_target_properties(hello_plugin PROPERTIES PREFIX "") # Optional: remove 'lib' prefix on some systems

# On Windows, MODULE libraries are .dll, on Linux .so, on macOS .dylib
# CMake handles this automatically.

# --- Echo Plugin ---
add_library(echo_plugin MODULE echo_plugin.cpp)
set_target_properties(echo_plugin PROPERTIES PREFIX "") # Optional: remove 'lib' prefix

# --- Installation (Example) ---
# This defines where the plugins get installed if 'make install' is run.
# The main application can then look for plugins in this standard location.
# Adjust DESTINATION path as per your project structure.
# Example: install them into a 'plugins' subdirectory within the main build/install lib directory
# install(TARGETS hello_plugin echo_plugin
#         LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/neurodeck/plugins
#         RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/neurodeck/plugins) # For Windows DLLs

# Note on Linking:
# These simple plugins (HelloCommand, EchoCommand) might not strictly need to *link*
# against a compiled shell library if Command is a simple abstract base class
# defined entirely in headers. However, if Command or other utilities they use
# had compiled components, linking would be necessary:
#
# Example if 'shell_lib' target exists and provides Command interface:
# target_link_libraries(hello_plugin PRIVATE shell_lib)
# target_link_libraries(echo_plugin PRIVATE shell_lib)
#
# For now, direct include_directories should suffice for resolving headers.
# The main project's CMakeLists.txt will be responsible for ensuring that
# any necessary libraries (like a potential shell_lib) are built before these plugins.
# If the shell components are header-only or an interface library, includes are enough.

# Suggestion for main CMakeLists.txt:
# If the main project has targets like 'shell' or 'core' (representing libraries),
# it's better to use target_include_directories:
#
# In main CMakeLists.txt, after defining shell_lib:
# target_include_directories(shell_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/shell)
#
# Then in this plugins/CMakeLists.txt:
# target_link_libraries(hello_plugin PRIVATE shell_lib) # This would also bring includes.
#
# This approach is more robust. The current include_directories() is a simpler
# fallback for when such targets aren't explicitly propagated.
# The current setup assumes the main CMake will add_subdirectory(plugins)
# and headers will be resolvable.
