cmake_minimum_required(VERSION 3.10)

# Project Name and Version
project(Neurodeck VERSION 0.1 LANGUAGES CXX)

# Set the C++ standard (using C++17 as a baseline)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build with debugging symbols by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Define output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include subdirectories
add_subdirectory(core)
add_subdirectory(shell)
add_subdirectory(desktop)
# You can later add GUI/desktop environment modules:
# add_subdirectory(gui)
# add_subdirectory(ide)

# Enable testing
include(CTest)
if(BUILD_TESTING)
    # Fetch GoogleTest from GitHub at a specified version (e.g., release-1.12.1)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    )
    # For Windows: prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Add the tests subdirectory
    add_subdirectory(tests)
endif()
