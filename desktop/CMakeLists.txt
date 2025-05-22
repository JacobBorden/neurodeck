# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Create a library or executable for the compositor scaffold
enable_language(C)
project(neurodeck_wayland LANGUAGES C CXX)

# Find Wayland server library
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_SERVER REQUIRED wayland-server)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLESV2 REQUIRED glesv2)
pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)

# Include directories
include_directories(
    ${WAYLAND_SERVER_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${GLESV2_INCLUDE_DIRS}
    ${WAYLAND_EGL_INCLUDE_DIRS}
)

# Add the compositor stub executable
add_executable(neurodeck_compositor
    main.cpp
)

target_link_libraries(neurodeck_compositor
    ${WAYLAND_SERVER_LIBRARIES}
    ${EGL_LIBRARIES}
    ${GLESV2_LIBRARIES}
    ${WAYLAND_EGL_LIBRARIES}
)

target_compile_options(neurodeck_compositor PRIVATE
    ${WAYLAND_SERVER_CFLAGS_OTHER}
)

# Install (optional)
install(TARGETS neurodeck_compositor DESTINATION bin)