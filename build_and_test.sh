#!/bin/bash

# Exit immediately if any command fails.
set -e

BUILD_DIR="build"

# Remove the build directory if it exists to ensure a clean build.
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning existing build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory and navigate into it.
mkdir "$BUILD_DIR" && cd "$BUILD_DIR"

# Configure the project. Enable testing explicitly.
cmake .. -DBUILD_TESTING=ON

# Build the project.
cmake --build .

# Run all tests and show detailed output on failure.
ctest --output-on-failure
./bin/neurodeck_compositor