# shell/CMakeLists.txt

# Create a static library for the shell commands
add_library(shell STATIC
    command.cpp
    tokenize.cpp
    commands/ls.cpp
    commands/clear.cpp
    commands/help.cpp
    commands/exit.cpp
    commands/open.cpp
)

# Public include directory for consumers of 'shell'
target_include_directories(shell
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against the core library for shared utilities
target_link_libraries(shell
    PUBLIC core
)

# Build the shell executable
add_executable(neurodeck_shell
    main.cpp
)

# Link the executable with the shell library
target_link_libraries(neurodeck_shell
    PRIVATE shell
)

# Optionally define installation rules
install(TARGETS neurodeck_shell DESTINATION bin)
install(TARGETS shell DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/commands/
        DESTINATION include/shell/commands
        FILES_MATCHING PATTERN "*.hpp")