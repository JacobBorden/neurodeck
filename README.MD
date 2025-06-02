# Neurodeck

**Project Overview**

Neurodeck is a custom C++ based shell and desktop environment designed to replace traditional Bash. It features a modular command architecture, pluggable desktop components, a Wayland/EGL compositor stub, and a planned AI-enabled hybrid IDE. The codebase is organized into reusable libraries (`core`, `shell`, `desktop`) and supports rapid build/test cycles via CMake and GoogleTest.

---

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Project Structure](#project-structure)
- [Building & Testing](#building--testing)
- [Usage](#usage)
- [Plugin Development](#plugin-development)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- **Modular Command Shell**: Features a `CommandRegistry` for managing commands and supports dynamic loading/unloading of commands via plugins.
- **Dynamic Plugin System:** Load custom commands at runtime from shared libraries (.so/.dll). Includes `loadplugin` and `unloadplugin` commands for managing plugins.
- **Custom Tokenizer**: `tokenize` function extracted into its own module for reuse and testing.
- **CMake Build System**: Top-level and subdirectory CMake files orchestrate building `core`, `shell`, `desktop`, plugins, and test targets.
- **Unit Testing with GoogleTest**: Automated tests cover tokenizer, command dispatch, all built-in commands, and core file/config utilities.
- **Wayland/EGL Desktop Compositor Stub**: Early prototype for a custom Linux desktop environment using Wayland and OpenGL ES.
- **Roadmap**: Plans for a graphical, vim-like IDE and AI-powered command suggestions.

---

## Prerequisites

- **C++17** compatible compiler (Clang or GCC)
- **CMake** (version 3.10 or newer)
- **Git**
- **Python** (for any future scripting or AI integration)
- **Linux** (target platform for eventual Wayland compositor and dynamic loading of .so files)

---

## Project Structure

```
neurodeck/                      # Top-level project root
├── CMakeLists.txt              # Root CMake configuration
├── core/                       # Shared utility library
│   ├── CMakeLists.txt
│   ├── file_io.cpp
│   └── config_parser.cpp
├── shell/                      # Modular shell implementation
│   ├── CMakeLists.txt
│   ├── main.cpp                # REPL entrypoint
│   ├── command.hpp             # Command base class
│   ├── command.cpp             # (Now mostly empty, legacy build_registry removed)
│   ├── command_registry.hpp    # Header for CommandRegistry
│   ├── command_registry.cpp    # Implementation of CommandRegistry
│   ├── tokenize.hpp
│   ├── tokenize.cpp
│   └── commands/               # One file per built-in command
│       ├── help.cpp / help.hpp
│       ├── exit.cpp / exit.hpp
│       ├── open.cpp / open.hpp
│       ├── exec_command.cpp / exec_command.hpp
│       ├── lua_command.cpp / lua_command.hpp
│       ├── load_plugin_command.hpp (header-only)
│       └── unload_plugin_command.hpp (header-only)
            # ls and clear commands have been removed.
├── plugins/                    # Example plugins
│   ├── CMakeLists.txt          # CMake for building plugins
│   ├── hello_plugin.cpp
│   └── echo_plugin.cpp
├── tests/                      # Unit tests
│   ├── CMakeLists.txt
│   ├── test_main.cpp           # GoogleTest entrypoint
│   ├── test_tokenize.cpp
│   └── test_dispatch.cpp       # (And many other test_*.cpp files)
├── desktop/                    # Wayland/EGL compositor stub (temporarily excluded from default build)
│   ├── CMakeLists.txt
│   ├── compositor.cpp
│   └── window.cpp
└── build_and_test.sh           # Script to build & run tests
```

---

## Building & Testing

Use the provided script for a one‑step build and test cycle:

```bash
./build_and_test.sh
```

This will:
1. Clean and recreate the `build/` directory
2. Configure CMake with `BUILD_TESTING=ON`
3. Compile the `core`, `shell`, `runTests` targets, and any defined plugins
4. Execute all unit tests via CTest

To build without tests:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Executables will be placed in `build/bin/`, and plugins typically in `build/plugins/`.

### Continuous Integration (CI)

This project uses GitHub Actions for Continuous Integration. The CI pipeline is defined in `.github/workflows/main.yml` and automatically triggers on pushes and pull requests to the `main` and `development` branches. The CI pipeline performs the following checks:

1.  **Build:** Compiles the project on an Ubuntu environment.
2.  **Unit Tests:** Executes all unit tests using CTest.
3.  **Static Analysis:**
    *   Runs Cppcheck for static analysis.
    *   Runs Clang-Tidy for further static analysis and style checks.
4.  **Code Coverage:**
    *   Generates a code coverage report using gcov/lcov.
    *   Uploads the report to Codecov.
    *   **Note:** There is currently a known issue with the CMake configuration that prevents accurate coverage measurement for the `core` and `shell` libraries. This is being investigated. Once resolved, the CI will enforce a minimum of 80% code coverage for these components.

### Running Tests Locally

The `./build_and_test.sh` script is the recommended way to build and run all tests.

Alternatively, if you have already built the project (e.g., by running `cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON` and `cmake --build build`), you can run tests directly from the build directory:

```bash
cd build
ctest --output-on-failure
```

### Code Coverage Locally

To generate a local code coverage report (assuming the CMake coverage generation issue is resolved):

1.  Ensure CMake is configured with coverage enabled:
    ```bash
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
    ```
2.  Build the project:
    ```bash
    cmake --build build
    ```
3.  Run tests to generate coverage data:
    ```bash
    cd build && ctest && cd ..
    ```
4.  Generate the LCOV report:
    ```bash
    lcov --capture --directory build --output-file coverage.info --base-directory . --rc lcov_branch_coverage=1
    lcov --remove coverage.info '/usr/*' 'tests/*' 'plugins/*' 'desktop/*' 'build/_deps/*' --output-file coverage.info
    ```
5.  You can view the report by generating HTML output:
    ```bash
    genhtml coverage.info --output-directory out_coverage_report --rc lcov_branch_coverage=1
    ```
    Then open `out_coverage_report/index.html` in a browser.
    Or, use the script `scripts/check_coverage.py coverage.info 80 core/ shell/` to check coverage percentages for specific directories.

---

## Usage

After a successful build, launch your custom shell:

```bash
./build/bin/neurodeck_shell
```

In the REPL, try commands like:

- `help` — Show available commands
- `open <file>` — Opens a file (current behavior might be placeholder)
- `exec <executable> [args...]` — Executes an external program.
- `lua <script.lua> [args...]` or `lua "<code_string>"` — Executes a Lua script or code.
- `exit` — Quit the shell

**Directly Executing Files**

Neurodeck shell can directly execute programs and Lua scripts if you provide their path:
-   **Executable files:** Type the path to an executable (e.g., `./myprogram`, `/usr/bin/top`, `scripts/myscript.sh`). Arguments can be passed as usual (e.g., `./myprogram arg1 "another arg"`).
-   **Lua scripts:** Type the path to a Lua script ending in `.lua` (e.g., `scripts/myscript.lua`). Arguments are also supported (e.g., `scripts/myscript.lua param1`). The system must have `lua` installed and in its PATH for this to work.

This provides a convenient way to run local scripts or system utilities. For more explicit control or if you prefer the command syntax, the `exec` and `lua` commands are also available.

**Plugin Management**
- `loadplugin <path/to/plugin.[so|dll]>` — Loads a plugin.
- `unloadplugin <path/to/plugin.[so|dll]>` — Unloads a plugin.

### `exec` Command

The `exec` command allows you to run external system programs and scripts directly from the shell. It uses the system's underlying shell to execute the command.

**Usage:**
```
exec <command-to-run> [argument1] [argument2] ...
```

**Example:**
To list files in the `/tmp` directory:
```
neurodeck> exec ls -l /tmp
(Output from ls -l /tmp will be displayed here)
```

### `lua` Command

The `lua` command allows you to execute Lua scripts or Lua code strings directly from the Neurodeck shell.

**Usage:**
To execute a Lua script file:
```
lua <path_to_script.lua> [argument1] [argument2] ...
```
To execute a Lua code string:
```
lua "<lua_code_string>"
```

**Examples:**
Execute a script:
```
neurodeck> lua scripts/myscript.lua param1
(Output from myscript.lua with param1 will be displayed here)
```
Execute a code string:
```
neurodeck> lua "print('Hello from Neurodeck Lua!')"
Hello from Neurodeck Lua!
```
The Lua environment includes standard Lua libraries. If `shell.run` is available in Lua (provided by `LuaCommand`), it can be used to call other Neurodeck commands or external programs (e.g., `lua "shell.run('help')"` or `lua "shell.run('exec /bin/echo hello')"`).

---

## Plugin Development

Neurodeck supports dynamically loaded plugins (shared libraries) that can register new commands. Here's how to create one:

1.  **Include Headers:**
    Your plugin C++ file will need to include:
    ```cpp
    #include "<path_to_project>/shell/command.hpp"         // For Neurodeck::Command
    #include "<path_to_project>/shell/command_registry.hpp" // For Neurodeck::CommandRegistry
    #include <memory> // For std::make_unique
    ```
    Replace `<path_to_project>` with the appropriate relative path from your plugin source to the shell directory (e.g., `../shell/`).

2.  **Define Your Command Class:**
    Create a class that inherits from `Neurodeck::Command`:
    ```cpp
    namespace MyPluginNamespace { // Optional: Use a namespace for your plugin's commands
    class MyCustomCommand : public Neurodeck::Command {
    public:
        std::string name() const override { return "mycmd"; }
        std::string description() const override { return "This is my custom command."; }
        void run(const std::vector<std::string>& args) override {
            // Your command logic here
            std::cout << "MyCustomCommand executed!" << std::endl;
        }
    };
    } // namespace MyPluginNamespace
    ```

3.  **Implement Plugin Entry/Exit Points:**
    Your plugin must export two C-style functions:
    ```cpp
    extern "C" __attribute__((visibility("default"))) // Use __declspec(dllexport) on Windows
    void register_commands(Neurodeck::CommandRegistry* registry) {
        registry->register_command(std::make_unique<MyPluginNamespace::MyCustomCommand>());
        // Register other commands from your plugin here
    }

    extern "C" __attribute__((visibility("default"))) // Use __declspec(dllexport) on Windows
    void unregister_commands(Neurodeck::CommandRegistry* registry) {
        registry->unregister_command("mycmd");
        // Unregister other commands
    }
    ```
    *Note on visibility/export:* `__attribute__((visibility("default")))` is for GCC/Clang. For Windows (MSVC), you'd typically use `__declspec(dllexport)` on the functions. The `CommandRegistry` handles platform differences for `dlopen`/`LoadLibrary`.

4.  **Build as a Shared Library:**
    Compile your plugin into a shared library (e.g., `my_plugin.so` on Linux, `my_plugin.dll` on Windows).
    You can add it to `plugins/CMakeLists.txt` similar to `hello_plugin`:
    ```cmake
    add_library(my_plugin MODULE my_plugin.cpp)
    # Potentially link against shell or core if needed
    # Ensure include directories are set up if your plugin is outside the plugins directory
    ```

5.  **Load in Neurodeck Shell:**
    Once built, you can load your plugin using the `loadplugin` command in the Neurodeck shell:
    `loadplugin path/to/your/my_plugin.so`

---

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/xyz`)
3. Make changes, add tests, update CMake as needed
4. Run `./build_and_test.sh` to ensure everything passes
5. Submit a pull request

Please follow the existing code style and commit message conventions.

### Testing and Code Coverage

-   Please ensure that new features or bug fixes include appropriate unit tests.
-   All tests must pass for a pull request to be considered.
-   We aim for at least 80% code coverage for the `core` and `shell` libraries. While the tooling for accurate measurement is currently being fixed, please write tests with this goal in mind. New commands or significant modules should be accompanied by tests.

---

## License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.
