# CMakeLists.txt for Shared Components Library Examples
cmake_minimum_required(VERSION 3.20)
project(SharedComponentsExamples)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find SDL3
find_package(SDL3 REQUIRED)

# Add shared components library (adjust path as needed)
set(SHARED_COMPONENTS_DIR "../../")
add_subdirectory(${SHARED_COMPONENTS_DIR} shared_components_build)

# Include directories
include_directories(${SHARED_COMPONENTS_DIR})

# Basic Example
add_executable(basic_example basic_example.c)
target_link_libraries(basic_example PRIVATE shared_components SDL3::SDL3)

# Text Demo
add_executable(text_demo text_demo.c)
target_link_libraries(text_demo PRIVATE shared_components SDL3::SDL3)

# Complete Example (if source exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/complete_example.c")
    add_executable(complete_example complete_example.c)
    target_link_libraries(complete_example PRIVATE shared_components SDL3::SDL3)
endif()

# UI Demo (if source exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ui_demo.c")
    add_executable(ui_demo ui_demo.c)
    target_link_libraries(ui_demo PRIVATE shared_components SDL3::SDL3)
endif()

# Palette Editor (if source exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/palette_editor.c")
    add_executable(palette_editor palette_editor.c)
    target_link_libraries(palette_editor PRIVATE shared_components SDL3::SDL3)
endif()

# Set executable output directory
set_target_properties(basic_example text_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# Copy example data files if they exist
file(GLOB DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.pal" "${CMAKE_CURRENT_SOURCE_DIR}/*.dat")
if(DATA_FILES)
    file(COPY ${DATA_FILES} DESTINATION ${CMAKE_BINARY_DIR}/examples)
endif()

# Print build information
message(STATUS "Building Shared Components Library Examples")
message(STATUS "Examples will be built in: ${CMAKE_BINARY_DIR}/examples")
