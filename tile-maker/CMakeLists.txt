cmake_minimum_required(VERSION 3.21)

# Project configuration
project(TileMaker VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS ON)
elseif(APPLE)
    set(PLATFORM_MACOS ON)
elseif(UNIX)
    set(PLATFORM_LINUX ON)
endif()

# Toolchain detection and compatibility checking
if(WIN32)
    # Detect current toolchain
    if(MSVC)
        set(CURRENT_TOOLCHAIN "MSVC")
        message(STATUS "TileMaker: Using MSVC toolchain")
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
        set(CURRENT_TOOLCHAIN "MinGW")
        message(STATUS "TileMaker: Using MinGW toolchain")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(CURRENT_TOOLCHAIN "Clang")
        message(STATUS "TileMaker: Using Clang toolchain")
    else()
        set(CURRENT_TOOLCHAIN "Unknown")
        message(WARNING "TileMaker: Unknown toolchain detected")
    endif()
endif()

# Compiler-specific flags
if(MSVC)
    # Microsoft Visual C++ compiler
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX- /std:c11")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi /RTC1")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
    # Set Windows subsystem to console
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # GCC or Clang compiler
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

    # Platform-specific threading
    if(PLATFORM_WINDOWS)
        # Windows with GCC (MinGW)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    else()
        # Linux/macOS
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    endif()
endif()

# Set module path for custom Find modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/")

# Check if shared_components target exists, if not, add the shared directory
if(NOT TARGET shared_components)
    message(STATUS "TileMaker: Adding shared components library")
    add_subdirectory(../shared shared_build)
endif()

# Find SDL3 with toolchain compatibility checking
find_package(SDL3 REQUIRED)

# Toolchain compatibility validation on Windows
if(WIN32 AND SDL3_FOUND)
    if(DEFINED SDL3_TOOLCHAIN_COMPATIBLE AND NOT SDL3_TOOLCHAIN_COMPATIBLE)
        message(WARNING "================================================")
        message(WARNING "TOOLCHAIN COMPATIBILITY WARNING")
        message(WARNING "================================================")
        message(WARNING "Current toolchain: ${CURRENT_TOOLCHAIN}")
        message(WARNING "SDL3 may not be compatible with current toolchain")
        message(WARNING "This could cause compilation errors")
        message(WARNING "")
        message(WARNING "Recommended solutions:")
        if(CURRENT_TOOLCHAIN STREQUAL "MSVC")
            message(WARNING "- Install MSVC-compatible SDL3 (e.g., via vcpkg)")
            message(WARNING "- Use: vcpkg install sdl3:x64-windows")
        elseif(CURRENT_TOOLCHAIN STREQUAL "MinGW")
            message(WARNING "- Install MinGW-compatible SDL3")
            message(WARNING "- Use: pacman -S mingw-w64-x86_64-SDL3")
        endif()
        message(WARNING "================================================")
    else()
        message(STATUS "TileMaker: SDL3 toolchain compatibility OK")
    endif()
endif()

# Include directories

# Source files
set(SOURCES
    main.c
    palette_io.c
    tiles_io.c
    tile_sheet.c
    pixel_editor.c
    ui.c
)

# Create executable
add_executable(TileMaker ${SOURCES})

target_include_directories(TileMaker PUBLIC
    ${SDL3_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/../shared"
)

# Set target properties
set_target_properties(TileMaker PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# Platform-specific configurations
if(PLATFORM_WINDOWS)
    # Windows-specific settings
    if(MSVC)
        # Enable parallel builds
        target_compile_options(TileMaker PRIVATE "/MP")
        # Set runtime library
        set_property(TARGET TileMaker PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    # Windows libraries
    target_link_libraries(TileMaker
        shared_components
        ${SDL3_LIBRARIES}
        user32
        gdi32
        winmm
        imm32
        ole32
        oleaut32
        version
        uuid
        advapi32
        setupapi
        shell32
    )

elseif(PLATFORM_MACOS)
    # macOS-specific settings
    set_target_properties(TileMaker PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/../cmake/MacOSXBundleInfo.plist.in"
    )

    # macOS frameworks
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    find_library(FORCEFEEDBACK_LIBRARY ForceFeedback)
    find_library(METAL_LIBRARY Metal)

    target_link_libraries(TileMaker
        shared_components
        ${SDL3_LIBRARIES}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${COREAUDIO_LIBRARY}
        ${AUDIOTOOLBOX_LIBRARY}
        ${FORCEFEEDBACK_LIBRARY}
        ${METAL_LIBRARY}
    )

elseif(PLATFORM_LINUX)
    # Linux-specific settings
    find_package(PkgConfig REQUIRED)

    # Find system libraries
    find_package(Threads REQUIRED)
    pkg_check_modules(ALSA alsa)
    pkg_check_modules(PULSE libpulse)

    target_link_libraries(TileMaker
        shared_components
        ${SDL3_LIBRARIES}
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )

    # Optional audio libraries
    if(ALSA_FOUND)
        target_link_libraries(TileMaker ${ALSA_LIBRARIES})
    endif()

    if(PULSE_FOUND)
        target_link_libraries(TileMaker ${PULSE_LIBRARIES})
    endif()
endif()

# Clean up SDL3_LIBRARIES string
string(STRIP "${SDL3_LIBRARIES}" SDL3_LIBRARIES)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Install targets
install(TARGETS TileMaker
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Print configuration summary
message(STATUS "")
message(STATUS "================================================")
message(STATUS "TileMaker Configuration Summary:")
message(STATUS "================================================")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
if(WIN32)
    message(STATUS "  Toolchain: ${CURRENT_TOOLCHAIN}")
    if(DEFINED SDL3_TOOLCHAIN_COMPATIBLE)
        if(SDL3_TOOLCHAIN_COMPATIBLE)
            message(STATUS "  SDL3 Compatibility: ✓ Compatible")
        else()
            message(STATUS "  SDL3 Compatibility: ⚠ WARNING - May be incompatible")
        endif()
    else()
        message(STATUS "  SDL3 Compatibility: ? Unknown")
    endif()
endif()
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "  SDL3 Libraries: ${SDL3_LIBRARIES}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "================================================")
message(STATUS "")
