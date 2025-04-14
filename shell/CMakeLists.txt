# Create an executable for the shell application
add_executable(neurodeck_shell main.cpp)

# Link the shell target with the core library
target_link_libraries(neurodeck_shell PRIVATE core)

# Optionally, specify any include directories
target_include_directories(neurodeck_shell PRIVATE ${CMAKE_SOURCE_DIR}/core)
