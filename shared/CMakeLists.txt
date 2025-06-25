# Shared Components Library
cmake_minimum_required(VERSION 3.20)

# Define the shared components library
add_library(shared_components STATIC
    # Main library file
    shared_components.c

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
    text_renderer
    ui_framework
    palette_manager
    sdl_framework
    utilities
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link SDL3 dependencies
target_link_libraries(shared_components PUBLIC SDL3::SDL3)

# Set C standard
set_property(TARGET shared_components PROPERTY C_STANDARD 11)
