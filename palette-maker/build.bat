@echo off
setlocal enabledelayedexpansion

REM Palette Maker Windows Build Script
REM Cross-platform build script for SDL3 Palette Maker on Windows

REM Colors for output (basic Windows console)
set "INFO=[INFO]"
set "SUCCESS=[SUCCESS]"
set "WARNING=[WARNING]"
set "ERROR=[ERROR]"

REM Default values
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=false"
set "INSTALL_AFTER=false"
set "GENERATOR="
set "SDL3_PATH="
set "JOBS=4"

REM Function to show usage
if "%1"=="--help" goto :show_usage
if "%1"=="-h" goto :show_usage
if "%1"=="/?" goto :show_usage

REM Parse command line arguments
:parse_args
if "%1"=="" goto :args_done

if "%1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if "%1"=="--release" (
    set "BUILD_TYPE=Release"
    shift
    goto :parse_args
)
if "%1"=="--clean" (
    set "CLEAN_BUILD=true"
    shift
    goto :parse_args
)
if "%1"=="--install" (
    set "INSTALL_AFTER=true"
    shift
    goto :parse_args
)
if "%1"=="--vs2022" (
    set "GENERATOR=Visual Studio 17 2022"
    shift
    goto :parse_args
)
if "%1"=="--vs2019" (
    set "GENERATOR=Visual Studio 16 2019"
    shift
    goto :parse_args
)
if "%1"=="--mingw" (
    set "GENERATOR=MinGW Makefiles"
    shift
    goto :parse_args
)
if "%1"=="--ninja" (
    set "GENERATOR=Ninja"
    shift
    goto :parse_args
)
if "%1"=="--sdl3-path" (
    set "SDL3_PATH=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="--jobs" (
    set "JOBS=%2"
    shift
    shift
    goto :parse_args
)

echo %ERROR% Unknown option: %1
goto :show_usage

:args_done

REM Print build configuration
echo %INFO% Palette Maker Build Configuration
echo   Build Type: %BUILD_TYPE%
echo   Clean Build: %CLEAN_BUILD%
echo   Install After: %INSTALL_AFTER%
echo   Generator: %GENERATOR%
echo   SDL3 Path: %SDL3_PATH%
echo   Parallel Jobs: %JOBS%
echo.

REM Check for required tools
echo %INFO% Checking build requirements...

cmake --version >nul 2>&1
if errorlevel 1 (
    echo %ERROR% CMake is required but not installed
    echo Install CMake from: https://cmake.org/install/
    exit /b 1
)

for /f "tokens=3" %%a in ('cmake --version') do (
    echo %INFO% Found CMake version: %%a
    goto :cmake_found
)
:cmake_found

REM Check for Ninja if specified
if "%GENERATOR%"=="Ninja" (
    ninja --version >nul 2>&1
    if errorlevel 1 (
        echo %ERROR% Ninja is required but not installed
        echo Install Ninja: https://ninja-build.org/
        exit /b 1
    )
    echo %INFO% Found Ninja build system
)

REM Platform detection
echo %INFO% Detected platform: Windows

REM Set default generator if not specified
if "%GENERATOR%"=="" (
    REM Try to detect Visual Studio
    where cl >nul 2>&1
    if not errorlevel 1 (
        set "GENERATOR=Visual Studio 17 2022"
    ) else (
        REM Try MinGW
        where gcc >nul 2>&1
        if not errorlevel 1 (
            set "GENERATOR=MinGW Makefiles"
        ) else (
            echo %WARNING% No compiler detected, using default Visual Studio generator
            set "GENERATOR=Visual Studio 17 2022"
        )
    )
    echo %INFO% Using default generator: !GENERATOR!
)

REM Create and enter build directory
set "BUILD_DIR=build"

if "%CLEAN_BUILD%"=="true" (
    echo %INFO% Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

if not exist "%BUILD_DIR%" (
    echo %INFO% Creating build directory...
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"

REM Prepare CMake arguments
set "CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%"

if not "%GENERATOR%"=="" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -G "%GENERATOR%""
)

if not "%SDL3_PATH%"=="" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DSDL3_PATH=%SDL3_PATH%"
)

REM Configure
echo %INFO% Configuring project...
echo %INFO% CMake command: cmake %CMAKE_ARGS% ..

cmake %CMAKE_ARGS% ..
if errorlevel 1 (
    echo %ERROR% CMake configuration failed
    exit /b 1
)

echo %SUCCESS% Configuration completed

REM Build
echo %INFO% Building project...
set "BUILD_ARGS=--build . --config %BUILD_TYPE%"

if %JOBS% gtr 1 (
    set "BUILD_ARGS=%BUILD_ARGS% --parallel %JOBS%"
)

echo %INFO% Build command: cmake %BUILD_ARGS%

cmake %BUILD_ARGS%
if errorlevel 1 (
    echo %ERROR% Build failed
    exit /b 1
)

echo %SUCCESS% Build completed successfully

REM Install if requested
if "%INSTALL_AFTER%"=="true" (
    echo %INFO% Installing...
    cmake --build . --target install
    if errorlevel 1 (
        echo %ERROR% Installation failed
        exit /b 1
    )
    echo %SUCCESS% Installation completed
)

REM Show results
echo %SUCCESS% Build Summary:
echo   Build Type: %BUILD_TYPE%
echo   Generator: %GENERATOR%

REM Find executable
set "EXECUTABLE="
if exist "bin\PaletteMaker.exe" (
    set "EXECUTABLE=bin\PaletteMaker.exe"
) else if exist "bin\%BUILD_TYPE%\PaletteMaker.exe" (
    set "EXECUTABLE=bin\%BUILD_TYPE%\PaletteMaker.exe"
) else if exist "bin\PaletteMaker" (
    set "EXECUTABLE=bin\PaletteMaker"
) else (
    set "EXECUTABLE=<not found>"
)

echo   Executable: %EXECUTABLE%

if not "%EXECUTABLE%"=="<not found>" (
    echo.
    echo %SUCCESS% To run Palette Maker:
    echo   cd %BUILD_DIR%
    echo   %EXECUTABLE%
    echo.
    echo %INFO% Or with a palette file:
    echo   %EXECUTABLE% my_palette.dat
) else (
    echo %WARNING% Executable not found at expected location
    echo %INFO% Check the bin\ directory for PaletteMaker.exe
)

echo %SUCCESS% Build script completed successfully!
goto :eof

:show_usage
echo Usage: %0 [OPTIONS]
echo.
echo Options:
echo   --debug              Build in Debug mode
echo   --release            Build in Release mode (default)
echo   --clean              Clean build directory before building
echo   --install            Install after building
echo   --vs2022             Use Visual Studio 2022 generator
echo   --vs2019             Use Visual Studio 2019 generator
echo   --mingw              Use MinGW Makefiles generator
echo   --ninja              Use Ninja generator
echo   --sdl3-path PATH     Specify SDL3 installation path
echo   --jobs N             Number of parallel build jobs (default: 4)
echo   --help               Show this help message
echo.
echo Examples:
echo   %0                          # Simple release build
echo   %0 --debug --clean          # Clean debug build
echo   %0 --vs2022 --install       # Build with VS2022 and install
echo   %0 --sdl3-path "C:\SDL3"    # Specify SDL3 location
goto :eof
