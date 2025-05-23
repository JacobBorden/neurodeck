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
- [Contributing](#contributing)
- [License](#license)

---

## Features

- **Modular Command Shell**: Shell commands live in separate modules under `shell/commands/`, built as a static library.
- **Custom Tokenizer**: `tokenize` function extracted into its own module for reuse and testing.
- **CMake Build System**: Top-level and subdirectory CMake files orchestrate building `core`, `shell`, `desktop`, and test targets.
- **Unit Testing with GoogleTest**: Automated tests cover tokenizer, command dispatch, all built-in commands, and core file/config utilities.
- **Wayland/EGL Desktop Compositor Stub**: Early prototype for a custom Linux desktop environment using Wayland and OpenGL ES.
- **Roadmap**: Plans for a graphical, vim-like IDE and AI-powered command suggestions.

---

## Prerequisites

- **C++17** compatible compiler (Clang or GCC)
- **CMake** (version 3.10 or newer)
- **Git**
- **Python** (for any future scripting or AI integration)
- **Linux** (target platform for eventual Wayland compositor)

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
│   ├── command.cpp             # Registry builder
│   ├── tokenize.hpp
│   ├── tokenize.cpp
│   └── commands/               # One file per built-in command
│       ├── ls.cpp
│       ├── clear.cpp
│       ├── help.cpp
│       ├── exit.cpp
│       └── open.cpp
├── tests/                      # Unit tests
│   ├── CMakeLists.txt
│   ├── test_main.cpp           # GoogleTest entrypoint
│   ├── test_tokenize.cpp
│   └── test_dispatch.cpp
├── desktop/                    # Wayland/EGL compositor stub
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
3. Compile the `core`, `shell`, and `runTests` targets
4. Execute all unit tests via CTest

To build without tests:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Executables will be placed in `build/bin/`.

---

## Usage

After a successful build, launch your custom shell:

```bash
./build/bin/neurodeck_shell
```

In the REPL, try commands like:

- `ls` — List available modules
- `clear` — Clear the screen
- `open notes` — (Future) Launch the notes module
- `exit` — Quit the shell

---

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/xyz`)
3. Make changes, add tests, update CMake as needed
4. Run `./build_and_test.sh` to ensure everything passes
5. Submit a pull request

Please follow the existing code style and commit message conventions.

---

## License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.

