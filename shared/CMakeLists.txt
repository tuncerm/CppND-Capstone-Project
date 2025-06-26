# Shared Components Library
cmake_minimum_required(VERSION 3.20)

# Define the shared components library
add_library(shared_components STATIC
    # Main library file
    shared_components.c

    # Configuration system
    config/config_manager.c

    # Error handling
    error_handler/error_handler.c

    # Text rendering system
    text_renderer/text_renderer.c
    text_renderer/font_data.c

    # UI framework components
    ui_framework/ui_button.c
    ui_framework/ui_primitives.c

    # Palette management
    palette_manager/palette_manager.c

    # SDL3 framework
    sdl_framework/sdl_context.c

    # Utilities
    utilities/double_click.c
    utilities/file_utils.c
)

# Include directories for shared components
target_include_directories(shared_components PUBLIC
    config
    error_handler
    text_renderer
    ui_framework
    palette_manager
    sdl_framework
    utilities
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set C standard
set_property(TARGET shared_components PROPERTY C_STANDARD 11)

# Find SDL3 if not already found
if(NOT TARGET SDL3::SDL3)
    find_package(SDL3 REQUIRED)
endif()

# Link SDL3 dependencies - handle both SDL3::SDL3 and raw SDL3_LIBRARIES
if(TARGET SDL3::SDL3)
    target_link_libraries(shared_components PUBLIC SDL3::SDL3)
elseif(SDL3_LIBRARIES)
    target_link_libraries(shared_components PUBLIC ${SDL3_LIBRARIES})
else()
    message(FATAL_ERROR "SDL3 not found - cannot build shared_components")
endif()

# Include SDL3 directories
if(SDL3_INCLUDE_DIRS)
    target_include_directories(shared_components PUBLIC ${SDL3_INCLUDE_DIRS})
endif()

# Add tests subdirectory if testing is enabled
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()
