# Create a static library for shared core utilities
add_library(core STATIC file_io.cpp config_parser.cpp)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
