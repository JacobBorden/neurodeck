# Create a static library for shared core utilities
add_library(core STATIC file_io.cpp config_parser.cpp)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
    message(STATUS "Applying --coverage to core library target using set_property.")
    set_property(TARGET core APPEND_STRING PROPERTY COMPILE_FLAGS " --coverage")
    # Also try to set link flags for completeness, though compile flags are key for .gcno
    set_property(TARGET core APPEND_STRING PROPERTY LINK_FLAGS " --coverage")
  endif()
endif()
