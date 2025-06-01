cmake_minimum_required(VERSION 3.10)

# Project Name and Version
project(Neurodeck VERSION 0.1 LANGUAGES CXX)

option(ENABLE_COVERAGE "Enable code coverage" OFF) # Ensure this is here or use existing one

if(ENABLE_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
    message(STATUS "Coverage enabled. Modifying CMAKE_CXX_FLAGS_DEBUG and CMAKE_C_FLAGS_DEBUG.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage") # For executables
    # Also ensure global flags have it, though build type flags are often more specific
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage") 
  else()
    message(WARNING "Coverage is not supported for this compiler.")
  endif()
endif()

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

# Find Lua
find_package(Lua)
if(LUA_FOUND)
    # Note: target_include_directories and target_link_libraries are preferred for specific targets.
    # However, if Lua headers/libs are needed by multiple targets (core, shell, potentially plugins),
    # using include_directories and link_libraries at this scope can be simpler,
    # or we add them to an INTERFACE library that other targets link to.
    # For now, let's make them available globally for simplicity as core and shell will need them.
    include_directories(${LUA_INCLUDE_DIR})
    # LUA_LIBRARIES will be used in core/CMakeLists.txt target_link_libraries
    message(STATUS "Lua found: TRUE, Version: ${LUA_VERSION_STRING}")
    message(STATUS "LUA_INCLUDE_DIR: ${LUA_INCLUDE_DIR}")
    message(STATUS "LUA_LIBRARIES: ${LUA_LIBRARIES}")
    # Add a compile definition to indicate Lua is available
    add_compile_definitions(LUA_FOUND_IN_CMAKE)
else()
    message(WARNING "Lua not found. Lua-dependent features will be disabled/stubbed.")
endif()

# Include subdirectories
add_subdirectory(core)
add_subdirectory(shell)
add_subdirectory(desktop) # Temporarily commented out to avoid wayland-server dependency
add_subdirectory(plugins) # Add the plugins subdirectory
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

    # Code Coverage (LCOV) - only if testing is enabled
    # Ensure runTests target in tests/CMakeLists.txt has coverage flags enabled for GCC/Clang
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        message(STATUS "lcov and genhtml found. Adding 'coverage_report' target.")

        # The directory for lcov --capture needs to point to where the .gcno and .gcda files are.
        # For a target like 'runTests' in the 'tests' subdirectory, this is typically:
        # ${CMAKE_BINARY_DIR}/tests/CMakeFiles/runTests.dir/
        # Or more generally, the CMAKE_CURRENT_BINARY_DIR of the target.
        # However, lcov works best if it scans the directory containing the source files that were compiled
        # with coverage flags, and it will find the .gcno files there.
        # The .gcda files are created at runtime in the same location as .gcno files.
        # Let's use CMAKE_BINARY_DIR as a base and specify the tests build directory.
        # A common practice is to use the build directory of the tests target.
        # For simplicity in a custom command, we might need to be careful about paths.
        # ${CMAKE_BINARY_DIR}/tests seems like a reasonable base for .gcno and .gcda files.
        # However, .gcno files are typically alongside object files.
        # Let's assume the build system places them within a subdirectory for the runTests target.
        # The base directory for lcov to scan for .gcno and .gcda files.
        # Pointing to CMAKE_BINARY_DIR allows it to scan all compiled subprojects.
        set(COVERAGE_DATA_DIR ${CMAKE_BINARY_DIR})
        # Output directory for HTML report, relative to CMAKE_BINARY_DIR
        set(COVERAGE_HTML_DIR ${CMAKE_BINARY_DIR}/coverage_html)
        set(COVERAGE_INFO_FILE ${CMAKE_BINARY_DIR}/coverage.info)
        set(COVERAGE_INFO_FILTERED_FILE ${CMAKE_BINARY_DIR}/coverage_filtered.info)

        add_custom_target(coverage_report
            COMMAND ${LCOV_EXECUTABLE} --capture --directory ${COVERAGE_DATA_DIR} --output-file ${COVERAGE_INFO_FILE} --rc lcov_branch_coverage=1 --ignore-errors mismatch,source
            COMMAND ${LCOV_EXECUTABLE} --remove ${COVERAGE_INFO_FILE} "/usr/include/*" --output-file ${COVERAGE_INFO_FILTERED_FILE} --rc lcov_branch_coverage=1
            COMMAND ${GENHTML_EXECUTABLE} ${COVERAGE_INFO_FILTERED_FILE} --output-directory ${COVERAGE_HTML_DIR} --branch-coverage --legend
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating LCOV code coverage report in ${COVERAGE_HTML_DIR}"
        )
        # Note: The 'coverage_report' target does not automatically run the tests.
        # The user must run the 'runTests' executable first to generate .gcda files.
        # Adding a dependency on 'runTests' target ensures it's built, but not run.
        add_dependencies(coverage_report runTests)

        message(STATUS "To generate coverage report: \n1. Run the 'runTests' executable from '${CMAKE_BINARY_DIR}/bin/runTests'.\n2. Run 'make coverage_report' (or your generator's equivalent).\n3. Open '${COVERAGE_HTML_DIR}/index.html'.")

    else()
        message(WARNING "lcov or genhtml not found. 'coverage_report' target will not be available. Install lcov to generate coverage reports.")
    endif()

    # Documentation for Code Coverage:
    #
    # 1. Prerequisites:
    #    - Ensure you are using GCC or Clang compiler.
    #    - Install 'lcov' (which usually includes 'genhtml'). On Debian/Ubuntu: sudo apt-get install lcov
    #
    # 2. Build with Coverage Enabled:
    #    - The 'runTests' target in 'tests/CMakeLists.txt' is automatically configured with coverage flags
    #      ('--coverage') when using GCC or Clang.
    #    - Configure and build the project, ensuring tests are enabled (default).
    #      Example: cmake -S . -B build && cmake --build build
    #
    # 3. Generate Coverage Data (.gcda files):
    #    - After building, run the test executable. This generates the .gcda files containing execution counts.
    #      Example: ./build/bin/runTests
    #
    # 4. Generate HTML Report:
    #    - Run the 'coverage_report' make target (or your generator's equivalent for this custom target).
    #      Example: cmake --build build --target coverage_report
    #    - This will use 'lcov' to process the .gcno (compile-time) and .gcda (run-time) files,
    #      filter out system library coverage, and then use 'genhtml' to create an HTML report.
    #
    # 5. View Report:
    #    - Open the generated HTML report in your browser.
    #      Location: build/coverage_html/index.html (relative to your build directory)
    #
endif()
