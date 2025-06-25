cmake_minimum_required(VERSION 3.21)

# Project configuration
project(PaletteMaker VERSION 1.0.0 LANGUAGES C)

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

# Compiler-specific flags
if(MSVC)
    # Microsoft Visual C++ compiler
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX-")
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

# Set module path for custom Find modules (use parent's cmake modules)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/")

# Check if shared_components target exists, if not, add the shared directory
if(NOT TARGET shared_components)
    message(STATUS "PaletteMaker: Adding shared components library")
    add_subdirectory(../shared shared_build)
endif()

# Find SDL3 with cross-platform support
find_package(SDL3 REQUIRED)

# Include directories
include_directories(${SDL3_INCLUDE_DIRS})
include_directories(.)
include_directories(../shared)

# Source files
set(SOURCES
    main.c
    palette.c
    ui.c
)

# Create executable
add_executable(PaletteMaker ${SOURCES})

# Set target properties
set_target_properties(PaletteMaker PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# Platform-specific configurations
if(PLATFORM_WINDOWS)
    # Windows-specific settings
    if(MSVC)
        # Enable parallel builds
        target_compile_options(PaletteMaker PRIVATE "/MP")
        # Set runtime library
        set_property(TARGET PaletteMaker PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    # Windows libraries
    target_link_libraries(PaletteMaker
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
    set_target_properties(PaletteMaker PROPERTIES
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

    target_link_libraries(PaletteMaker
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

    target_link_libraries(PaletteMaker
        shared_components
        ${SDL3_LIBRARIES}
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )

    # Optional audio libraries
    if(ALSA_FOUND)
        target_link_libraries(PaletteMaker ${ALSA_LIBRARIES})
    endif()

    if(PULSE_FOUND)
        target_link_libraries(PaletteMaker ${PULSE_LIBRARIES})
    endif()
endif()

# Clean up SDL3_LIBRARIES string
string(STRIP "${SDL3_LIBRARIES}" SDL3_LIBRARIES)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Install targets
install(TARGETS PaletteMaker
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Print configuration summary
message(STATUS "")
message(STATUS "PaletteMaker Configuration Summary:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "  SDL3 Libraries: ${SDL3_LIBRARIES}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
