cmake_minimum_required(VERSION 3.21)

project(AssetComposer VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(WIN32)
    set(PLATFORM_WINDOWS ON)
elseif(APPLE)
    set(PLATFORM_MACOS ON)
elseif(UNIX)
    set(PLATFORM_LINUX ON)
endif()

if(MSVC)
    if(ENABLE_WARNINGS_AS_ERRORS)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX-")
    endif()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi /RTC1")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
    if(ENABLE_WARNINGS_AS_ERRORS)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    endif()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
    if(PLATFORM_WINDOWS)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/")

if(NOT TARGET shared_components)
    add_subdirectory(../shared shared_build)
endif()

find_package(SDL3 REQUIRED)

set(SOURCES
    main.c
    asset_db.c
    ../tile-maker/palette_io.c
    ../tile-maker/tiles_io.c
    ../tile-maker/tile_specs_io.c
)

add_executable(AssetComposer ${SOURCES})

target_include_directories(AssetComposer PUBLIC
    ${SDL3_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/../shared"
    "${CMAKE_CURRENT_SOURCE_DIR}/../tile-maker"
)

set_target_properties(AssetComposer PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

if(PLATFORM_WINDOWS)
    if(MSVC)
        target_compile_options(AssetComposer PRIVATE "/MP")
        set_property(TARGET AssetComposer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    target_link_libraries(AssetComposer
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
    set_target_properties(AssetComposer PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/../cmake/MacOSXBundleInfo.plist.in"
    )

    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    find_library(FORCEFEEDBACK_LIBRARY ForceFeedback)
    find_library(METAL_LIBRARY Metal)

    target_link_libraries(AssetComposer
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
    find_package(PkgConfig REQUIRED)
    find_package(Threads REQUIRED)
    pkg_check_modules(ALSA alsa)
    pkg_check_modules(PULSE libpulse)

    target_link_libraries(AssetComposer
        shared_components
        ${SDL3_LIBRARIES}
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )

    if(ALSA_FOUND)
        target_link_libraries(AssetComposer ${ALSA_LIBRARIES})
    endif()
    if(PULSE_FOUND)
        target_link_libraries(AssetComposer ${PULSE_LIBRARIES})
    endif()
endif()
