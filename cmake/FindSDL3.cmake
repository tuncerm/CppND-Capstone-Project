# FindSDL3
# --------
#
# Locate SDL3 library with robust toolchain compatibility detection
#
# This module defines:
#  SDL3_FOUND - True if SDL3 was found
#  SDL3_INCLUDE_DIRS - Include directories for SDL3
#  SDL3_LIBRARIES - Link libraries for SDL3
#  SDL3_VERSION_STRING - Version of SDL3 found
#  SDL3_TOOLCHAIN_COMPATIBLE - True if found SDL3 matches current toolchain
#
# This module accepts the following variables:
#  SDL3_PATH - Can be set to SDL3 install path or Windows build path
#  SDL3_FORCE_TOOLCHAIN - Force specific toolchain (MSVC, MinGW, etc)

# Copyright (c) 2025, Cross-Platform Build Configuration
# Distributed under the OSI-approved BSD License

if(NOT SDL3_FOUND)

# Detect current toolchain
set(SDL3_CURRENT_TOOLCHAIN "UNKNOWN")
if(MSVC)
    set(SDL3_CURRENT_TOOLCHAIN "MSVC")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    if(WIN32)
        set(SDL3_CURRENT_TOOLCHAIN "MinGW")
    else()
        set(SDL3_CURRENT_TOOLCHAIN "GCC")
    endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(SDL3_CURRENT_TOOLCHAIN "Clang")
endif()

message(STATUS "SDL3 Finder: Detected toolchain: ${SDL3_CURRENT_TOOLCHAIN}")

# Set up platform-specific search paths with toolchain prioritization
set(_SDL3_SEARCH_PATHS)
set(_SDL3_PRIORITY_PATHS)

# Common installation directories
list(APPEND _SDL3_SEARCH_PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

if(WIN32)
  # Windows-specific paths with toolchain prioritization
  if(SDL3_CURRENT_TOOLCHAIN STREQUAL "MSVC")
    # Prioritize MSVC-compatible paths
    list(APPEND _SDL3_PRIORITY_PATHS
      $ENV{SDL3DIR}
      $ENV{SDL3_DIR}
      "C:/SDL3"
      "C:/Program Files/SDL3"
      "C:/Program Files (x86)/SDL3"
      "${CMAKE_SOURCE_DIR}/../SDL3"
      "C:/vcpkg/installed/x64-windows"
      "C:/vcpkg/installed/x86-windows"
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SDL Development Team\\SDL 3.0.0;InstallPath]"
    )
    # MinGW paths as fallback
    list(APPEND _SDL3_SEARCH_PATHS
      "C:/msys64/mingw64"
      "C:/msys64/mingw32"
      "C:/MinGW/msys/1.0"
    )
  elseif(SDL3_CURRENT_TOOLCHAIN STREQUAL "MinGW")
    # Prioritize MinGW-compatible paths
    list(APPEND _SDL3_PRIORITY_PATHS
      "C:/msys64/mingw64"
      "C:/msys64/mingw32"
      "C:/MinGW/msys/1.0"
      $ENV{SDL3DIR}
      $ENV{SDL3_DIR}
    )
    # MSVC paths as fallback
    list(APPEND _SDL3_SEARCH_PATHS
      "C:/SDL3"
      "C:/Program Files/SDL3"
      "C:/Program Files (x86)/SDL3"
      "${CMAKE_SOURCE_DIR}/../SDL3"
    )
  else()
    # Default Windows paths
    list(APPEND _SDL3_SEARCH_PATHS
      $ENV{SDL3DIR}
      $ENV{SDL3_DIR}
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SDL Development Team\\SDL 3.0.0;InstallPath]"
      "C:/SDL3"
      "C:/Program Files/SDL3"
      "C:/Program Files (x86)/SDL3"
      "${CMAKE_SOURCE_DIR}/../SDL3"
      "C:/msys64/mingw64"
      "C:/msys64/mingw32"
      "C:/vcpkg/installed/x64-windows"
      "C:/vcpkg/installed/x86-windows"
    )
  endif()
endif()

# Custom path from user
if(SDL3_PATH)
  list(INSERT _SDL3_PRIORITY_PATHS 0 ${SDL3_PATH})
endif()

# Combine priority and search paths
list(APPEND _SDL3_SEARCH_PATHS ${_SDL3_PRIORITY_PATHS})

# Architecture detection for Windows
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_SDL3_ARCH_SUFFIX "x64")
else()
  set(_SDL3_ARCH_SUFFIX "x86")
endif()

# Function to detect toolchain compatibility of SDL3 installation
function(check_sdl3_toolchain_compatibility SDL3_INCLUDE_PATH SDL3_LIB_PATH RESULT_VAR)
  set(${RESULT_VAR} FALSE PARENT_SCOPE)

  if(NOT SDL3_INCLUDE_PATH OR NOT SDL3_LIB_PATH)
    return()
  endif()

  # Check if path indicates MinGW installation
  string(FIND "${SDL3_INCLUDE_PATH}" "msys64" MSYS_FOUND)
  string(FIND "${SDL3_INCLUDE_PATH}" "mingw" MINGW_FOUND)
  string(FIND "${SDL3_LIB_PATH}" "msys64" MSYS_LIB_FOUND)
  string(FIND "${SDL3_LIB_PATH}" "mingw" MINGW_LIB_FOUND)

  set(IS_MINGW_PATH FALSE)
  if(MSYS_FOUND GREATER -1 OR MINGW_FOUND GREATER -1 OR MSYS_LIB_FOUND GREATER -1 OR MINGW_LIB_FOUND GREATER -1)
    set(IS_MINGW_PATH TRUE)
  endif()

  # Check compatibility
  if(SDL3_CURRENT_TOOLCHAIN STREQUAL "MSVC" AND NOT IS_MINGW_PATH)
    set(${RESULT_VAR} TRUE PARENT_SCOPE)
    message(STATUS "SDL3 Finder: Found MSVC-compatible SDL3 at ${SDL3_INCLUDE_PATH}")
  elseif(SDL3_CURRENT_TOOLCHAIN STREQUAL "MinGW" AND IS_MINGW_PATH)
    set(${RESULT_VAR} TRUE PARENT_SCOPE)
    message(STATUS "SDL3 Finder: Found MinGW-compatible SDL3 at ${SDL3_INCLUDE_PATH}")
  elseif(NOT WIN32)
    # On non-Windows platforms, assume compatibility
    set(${RESULT_VAR} TRUE PARENT_SCOPE)
  endif()
endfunction()

# Find include directory with toolchain compatibility check
set(SDL3_INCLUDE_DIR)
set(SDL3_TOOLCHAIN_COMPATIBLE FALSE)

foreach(SEARCH_PATH ${_SDL3_PRIORITY_PATHS} ${_SDL3_SEARCH_PATHS})
  find_path(SDL3_INCLUDE_DIR_TEMP
    NAMES SDL3/SDL.h SDL.h
    HINTS ${SEARCH_PATH}
    PATH_SUFFIXES include SDL3 include/SDL3
    NO_DEFAULT_PATH
  )

  if(SDL3_INCLUDE_DIR_TEMP)
    # Check toolchain compatibility
    check_sdl3_toolchain_compatibility("${SDL3_INCLUDE_DIR_TEMP}" "${SEARCH_PATH}" IS_COMPATIBLE)

    if(IS_COMPATIBLE OR NOT WIN32)
      set(SDL3_INCLUDE_DIR ${SDL3_INCLUDE_DIR_TEMP})
      set(SDL3_TOOLCHAIN_COMPATIBLE TRUE)
      message(STATUS "SDL3 Finder: Using toolchain-compatible SDL3 from ${SEARCH_PATH}")
      break()
    else()
      message(STATUS "SDL3 Finder: Found incompatible SDL3 at ${SEARCH_PATH} (toolchain: ${SDL3_CURRENT_TOOLCHAIN})")
    endif()
  endif()

  unset(SDL3_INCLUDE_DIR_TEMP CACHE)
endforeach()

# If no compatible SDL3 found, fall back to any available SDL3
if(NOT SDL3_INCLUDE_DIR)
  message(WARNING "SDL3 Finder: No toolchain-compatible SDL3 found, falling back to any available SDL3...")

  find_path(SDL3_INCLUDE_DIR
    NAMES SDL3/SDL.h SDL.h
    HINTS
      ENV SDL3DIR
      ENV SDL3_DIR
      ${SDL3_PATH}
    PATH_SUFFIXES include SDL3 include/SDL3
    PATHS ${_SDL3_SEARCH_PATHS}
  )
endif()

# Find main SDL3 library with toolchain compatibility
set(SDL3_LIBRARY_TEMP)

if(SDL3_INCLUDE_DIR)
  # Get the base path from include directory
  get_filename_component(SDL3_BASE_PATH "${SDL3_INCLUDE_DIR}" DIRECTORY)

  if(WIN32)
    # Windows library names and paths
    find_library(SDL3_LIBRARY_TEMP
      NAMES SDL3 SDL3-static libSDL3 libSDL3-static
      HINTS ${SDL3_BASE_PATH}
      PATH_SUFFIXES
        lib lib/${_SDL3_ARCH_SUFFIX}
        lib/x64 lib/x86
        ${_SDL3_ARCH_SUFFIX}
      NO_DEFAULT_PATH
    )

    # Find SDL3main library for Windows
    find_library(SDL3MAIN_LIBRARY
      NAMES SDL3main SDL3main-static libSDL3main libSDL3main-static
      HINTS ${SDL3_BASE_PATH}
      PATH_SUFFIXES
        lib lib/${_SDL3_ARCH_SUFFIX}
        lib/x64 lib/x86
        ${_SDL3_ARCH_SUFFIX}
      NO_DEFAULT_PATH
    )
  else()
    # Unix/Linux/macOS library finding
    find_library(SDL3_LIBRARY_TEMP
      NAMES SDL3 libSDL3
      HINTS ${SDL3_BASE_PATH}
      PATH_SUFFIXES lib lib64 lib32
      NO_DEFAULT_PATH
    )
  endif()
endif()

# Fallback library search if not found in compatible location
if(NOT SDL3_LIBRARY_TEMP)
  if(WIN32)
    find_library(SDL3_LIBRARY_TEMP
      NAMES SDL3 SDL3-static libSDL3 libSDL3-static
      HINTS
        ENV SDL3DIR
        ENV SDL3_DIR
        ${SDL3_PATH}
      PATH_SUFFIXES
        lib lib/${_SDL3_ARCH_SUFFIX}
        lib/x64 lib/x86
        ${_SDL3_ARCH_SUFFIX}
      PATHS ${_SDL3_SEARCH_PATHS}
    )

    find_library(SDL3MAIN_LIBRARY
      NAMES SDL3main SDL3main-static libSDL3main libSDL3main-static
      HINTS
        ENV SDL3DIR
        ENV SDL3_DIR
        ${SDL3_PATH}
      PATH_SUFFIXES
        lib lib/${_SDL3_ARCH_SUFFIX}
        lib/x64 lib/x86
        ${_SDL3_ARCH_SUFFIX}
      PATHS ${_SDL3_SEARCH_PATHS}
    )
  else()
    find_library(SDL3_LIBRARY_TEMP
      NAMES SDL3 libSDL3
      HINTS
        ENV SDL3DIR
        ENV SDL3_DIR
        ${SDL3_PATH}
      PATH_SUFFIXES lib lib64 lib32
      PATHS ${_SDL3_SEARCH_PATHS}
    )
  endif()
endif()

# Handle framework on macOS
if(APPLE AND NOT SDL3_LIBRARY_TEMP)
  find_library(SDL3_LIBRARY_TEMP
    NAMES SDL3
    HINTS
      ENV SDL3DIR
      ENV SDL3_DIR
      ${SDL3_PATH}
    PATHS
      ~/Library/Frameworks
      /Library/Frameworks
      /System/Library/Frameworks
    PATH_SUFFIXES Frameworks
  )
endif()

# Version detection
if(SDL3_INCLUDE_DIR)
  # Try to find version in SDL_version.h
  if(EXISTS "${SDL3_INCLUDE_DIR}/SDL3/SDL_version.h")
    set(SDL3_VERSION_H "${SDL3_INCLUDE_DIR}/SDL3/SDL_version.h")
  elseif(EXISTS "${SDL3_INCLUDE_DIR}/SDL_version.h")
    set(SDL3_VERSION_H "${SDL3_INCLUDE_DIR}/SDL_version.h")
  endif()

  if(SDL3_VERSION_H)
    file(STRINGS "${SDL3_VERSION_H}" SDL3_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL3_VERSION_H}" SDL3_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL3_VERSION_H}" SDL3_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")

    if(SDL3_VERSION_MAJOR_LINE)
      string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL3_VERSION_MAJOR "${SDL3_VERSION_MAJOR_LINE}")
    endif()
    if(SDL3_VERSION_MINOR_LINE)
      string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL3_VERSION_MINOR "${SDL3_VERSION_MINOR_LINE}")
    endif()
    if(SDL3_VERSION_PATCH_LINE)
      string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL3_VERSION_PATCH "${SDL3_VERSION_PATCH_LINE}")
    endif()

    if(SDL3_VERSION_MAJOR AND SDL3_VERSION_MINOR AND SDL3_VERSION_PATCH)
      set(SDL3_VERSION_STRING "${SDL3_VERSION_MAJOR}.${SDL3_VERSION_MINOR}.${SDL3_VERSION_PATCH}")
    endif()
  endif()
endif()

# Set up the libraries list
set(SDL3_LIBRARIES)
set(SDL3_INCLUDE_DIRS)

if(SDL3_LIBRARY_TEMP)
  list(APPEND SDL3_LIBRARIES ${SDL3_LIBRARY_TEMP})

  # Add platform-specific libraries
  if(WIN32)
    if(SDL3MAIN_LIBRARY)
      list(APPEND SDL3_LIBRARIES ${SDL3MAIN_LIBRARY})
    endif()

    # Add Windows system libraries
    list(APPEND SDL3_LIBRARIES
      user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32
    )
  elseif(APPLE)
    # Add macOS frameworks
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    find_library(FORCEFEEDBACK_LIBRARY ForceFeedback)
    find_library(METAL_LIBRARY Metal)

    if(COCOA_LIBRARY)
      list(APPEND SDL3_LIBRARIES ${COCOA_LIBRARY})
    endif()
    if(IOKIT_LIBRARY)
      list(APPEND SDL3_LIBRARIES ${IOKIT_LIBRARY})
    endif()
    if(COREVIDEO_LIBRARY)
      list(APPEND SDL3_LIBRARIES ${COREVIDEO_LIBRARY})
    endif()
    if(COREAUDIO_LIBRARY)
      list(APPEND SDL3_LIBRARIES ${COREAUDIO_LIBRARY})
    endif()
    if(AUDIOTOOLBOX_LIBRARY)
      list(APPEND SDL3_LIBRARIES ${AUDIOTOOLBOX_LIBRARY})
    endif()
    if(FORCEFEEDBACK_LIBRARY)
      list(APPEND SDL3_LIBRARIES ${FORCEFEEDBACK_LIBRARY})
    endif()
    if(METAL_LIBRARY)
      list(APPEND SDL3_LIBRARIES ${METAL_LIBRARY})
    endif()

  else()
    # Linux and other Unix systems
    find_package(Threads REQUIRED)
    list(APPEND SDL3_LIBRARIES Threads::Threads ${CMAKE_DL_LIBS} m)
  endif()
endif()

if(SDL3_INCLUDE_DIR)
  list(APPEND SDL3_INCLUDE_DIRS ${SDL3_INCLUDE_DIR})

  # Add SDL3 subdirectory if it exists
  if(EXISTS "${SDL3_INCLUDE_DIR}/SDL3")
    list(APPEND SDL3_INCLUDE_DIRS ${SDL3_INCLUDE_DIR}/SDL3)
  endif()
endif()

# Handle REQUIRED and QUIET arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL3
  REQUIRED_VARS SDL3_LIBRARIES SDL3_INCLUDE_DIRS
  VERSION_VAR SDL3_VERSION_STRING
)

if(SDL3_FOUND)
  # Warn about toolchain compatibility
  if(WIN32 AND NOT SDL3_TOOLCHAIN_COMPATIBLE)
    message(WARNING "SDL3 Finder: Found SDL3 but it may not be compatible with current toolchain (${SDL3_CURRENT_TOOLCHAIN})")
    message(WARNING "SDL3 Finder: This may cause compilation errors. Consider installing SDL3 for ${SDL3_CURRENT_TOOLCHAIN}")
  endif()

  # Create imported target for modern CMake usage
  if(NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 UNKNOWN IMPORTED)
    set_target_properties(SDL3::SDL3 PROPERTIES
      IMPORTED_LOCATION "${SDL3_LIBRARY_TEMP}"
      INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${SDL3_LIBRARIES}"
    )

    if(WIN32 AND SDL3MAIN_LIBRARY)
      add_library(SDL3::SDL3main UNKNOWN IMPORTED)
      set_target_properties(SDL3::SDL3main PROPERTIES
        IMPORTED_LOCATION "${SDL3MAIN_LIBRARY}"
      )
    endif()
  endif()
endif()

# Clean up
unset(SDL3_LIBRARY_TEMP)
unset(SDL3MAIN_LIBRARY)
unset(_SDL3_SEARCH_PATHS)
unset(_SDL3_PRIORITY_PATHS)
unset(_SDL3_ARCH_SUFFIX)

mark_as_advanced(SDL3_INCLUDE_DIR SDL3_LIBRARY_TEMP SDL3MAIN_LIBRARY)

endif()
