# Testing Framework for Shared Components Library
cmake_minimum_required(VERSION 3.20)

# Enable testing for this directory and subdirectories
enable_testing()

# Option to build tests
option(SHARED_COMPONENTS_BUILD_TESTS "Build shared_components tests" ON)

if(NOT SHARED_COMPONENTS_BUILD_TESTS)
    return()
endif()

# Find or download Google Test
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    message(STATUS "Google Test not found, downloading and building from source...")

    # Download and build Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    # Alias targets for consistency
    if(NOT TARGET GTest::gtest)
        add_library(GTest::gtest ALIAS gtest)
    endif()
    if(NOT TARGET GTest::gtest_main)
        add_library(GTest::gtest_main ALIAS gtest_main)
    endif()
else()
    message(STATUS "Using system Google Test")
endif()

# Test executable name
set(TEST_TARGET shared_components_tests)

# Test source files
set(TEST_SOURCES
    # Test main
    test_main.cpp

    # Unit tests
    unit/test_shared_components.cpp
    unit/test_text_renderer.cpp
    unit/test_ui_button.cpp
    unit/test_palette_manager.cpp
    unit/test_sdl_context.cpp
    unit/test_file_utils.cpp
    unit/test_double_click.cpp
    unit/test_config_manager.cpp

    # Integration tests
    integration/test_text_ui_integration.cpp
    integration/test_palette_rendering_integration.cpp
    integration/test_sdl_components_integration.cpp

    # Test utilities
    utils/test_helpers.cpp
)

# Create test executable
add_executable(${TEST_TARGET} ${TEST_SOURCES})

# Set C++ standard for tests
set_property(TARGET ${TEST_TARGET} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${TEST_TARGET} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${TEST_TARGET} PROPERTY CXX_STANDARD 17)

# Link against shared_components library and Google Test
target_link_libraries(${TEST_TARGET}
    PRIVATE
        shared_components
        GTest::gtest
        GTest::gtest_main
)

# Include directories
target_include_directories(${TEST_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        utils
)

# Platform-specific test configuration
if(WIN32)
    # Windows-specific test settings
    if(MSVC)
        target_compile_options(${TEST_TARGET} PRIVATE "/W4")
        # Set runtime library to match shared_components
        set_property(TARGET ${TEST_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
elseif(UNIX)
    # Unix-specific test settings
    target_compile_options(${TEST_TARGET} PRIVATE "-Wall" "-Wextra" "-Wpedantic")
    if(NOT APPLE)
        target_link_libraries(${TEST_TARGET} PRIVATE pthread)
    endif()
endif()

# Register tests with CTest
add_test(
    NAME ${TEST_TARGET}
    COMMAND ${TEST_TARGET}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Test discovery for individual test cases
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.10)
    include(GoogleTest)
    gtest_discover_tests(${TEST_TARGET}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif()

# Coverage target (if gcov/lcov available)
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        message(STATUS "Coverage tools found - enabling coverage target")

        # Add coverage flags
        target_compile_options(shared_components PRIVATE --coverage)
        target_link_options(shared_components PRIVATE --coverage)

        # Coverage target
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            COMMAND ${TEST_TARGET}
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/googletest/*' '*/tests/*' --output-file coverage.info.cleaned
            COMMAND ${GENHTML_PATH} -o coverage coverage.info.cleaned
            COMMAND ${CMAKE_COMMAND} -E remove coverage.info coverage.info.cleaned
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )

        add_dependencies(coverage ${TEST_TARGET})
    endif()
endif()

# Valgrind target (if available on Unix)
if(UNIX AND NOT APPLE)
    find_program(VALGRIND_PATH valgrind)
    if(VALGRIND_PATH)
        add_custom_target(valgrind
            COMMAND ${VALGRIND_PATH} --tool=memcheck --leak-check=full --show-leak-kinds=all
                    --track-origins=yes --verbose --log-file=valgrind-out.txt
                    ./${TEST_TARGET}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Running tests with Valgrind memory checker"
        )
        add_dependencies(valgrind ${TEST_TARGET})
    endif()
endif()

# Print test configuration summary
message(STATUS "")
message(STATUS "Shared Components Test Configuration:")
message(STATUS "  Build Tests: ${SHARED_COMPONENTS_BUILD_TESTS}")
message(STATUS "  Test Target: ${TEST_TARGET}")
if(GTest_FOUND)
    message(STATUS "  Google Test: System installation")
else()
    message(STATUS "  Google Test: Downloaded from source")
endif()
message(STATUS "  Test Sources: ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "")
