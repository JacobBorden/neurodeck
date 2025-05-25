# Changelog

All notable changes to the Neurodeck project will be documented in this file.

## [Unreleased]
- **Refactored & Enhanced Core Shell and Command System:**
  - Introduced `Neurodeck::CommandRegistry` for robust command management, replacing the old static registration.
  - Standardized `Neurodeck::Command` interface (namespaced, added `description()` method).
  - Updated all built-in commands to conform to the new interface and namespace.
  - Improved shell I/O handling and error reporting in `shell/main.cpp`.
- **Implemented Dynamic Plugin Architecture:**
  - Added capability to load and unload commands dynamically from shared library plugins.
  - Plugins must export `register_commands(Neurodeck::CommandRegistry*)` and `unregister_commands(Neurodeck::CommandRegistry*)`.
  - Introduced `loadplugin` and `unloadplugin` shell commands.
  - Created example `hello_plugin` and `echo_plugin`.
- **Core API and Module Loading Documentation:**
  - Added Doxygen-style comments to `core/config_parser.hpp`, `core/file_io.hpp`, and `shell/command_registry.hpp`.
- **Test Suite Overhaul:**
  - Refactored the entire test suite (`tests/`) to align with the new `CommandRegistry` architecture and namespaced commands.
  - Ensured all tests compile and accurately reflect the current codebase.
- **Build System:**
  - Updated CMake configurations to support plugin building.
  - Temporarily excluded the `desktop` subdirectory from builds to resolve unrelated compilation issues.
- **Added:** Wayland/EGL desktop compositor stub (`desktop/main.cpp`, `desktop/CMakeLists.txt`)
- **Added:** Unit tests for all built-in shell commands and core file/config utilities
- **Added:** `exec` command: Allows execution of external system commands (e.g., `/bin/ls`, commands in `$PATH`) and captures their `stdout` and `stderr`. Usage: `exec <command> [args...]`.
- **Planned:** Desktop environment stub integration
- **Planned:** AI-enabled command suggestions module
- **Planned:** Graphical IDE prototype

## [0.2.0] - 2025-04-23
### Added
- Modular command architecture: extracted built-in commands into `shell/commands/`
- `tokenize` function moved to its own module (`shell/tokenize.cpp`, `tokenize.hpp`)
- GoogleTest integration for unit tests (`tests/test_tokenize.cpp`, `tests/test_dispatch.cpp`)
- `build_and_test.sh` script to automate build and testing
- Enhanced CMake configurations for `shell/` and `tests/`

## [0.1.0] - 2025-04-22
### Added
- Initial Neurodeck shell REPL in C++ (`shell/main.cpp`)
- Basic commands: `ls`, `clear`, `help`, `open`, `exit`
- Core utility library placeholder (`core/`)
- Top-level CMakeLists and subdirectory CMake support
- Basic `.gitignore` and project scaffolding

<!--
### Format conventions:
- Entries under each version are categorized by Added, Changed, Fixed, Removed
- Versions are in descending chronological order
- Unreleased section for upcoming work
-->
