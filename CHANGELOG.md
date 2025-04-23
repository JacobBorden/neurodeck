# Changelog

All notable changes to the Neurodeck project will be documented in this file.

## [Unreleased]
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

