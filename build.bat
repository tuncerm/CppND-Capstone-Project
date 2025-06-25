@echo off
REM Cross-platform build script for Windows
REM Supports Visual Studio, MinGW, and Clang

setlocal enabledelayedexpansion

echo.
echo ============================================
echo  PlayGame - Cross-Platform Build (Windows)
echo ============================================
echo.

REM Set default values
set BUILD_TYPE=Release
set GENERATOR=
set CLEAN_BUILD=0
set INSTALL_TARGET=0
set SDL3_PATH=

REM Parse command line arguments
:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--debug" set BUILD_TYPE=Debug
if /i "%~1"=="--release" set BUILD_TYPE=Release
if /i "%~1"=="--clean" set CLEAN_BUILD=1
if /i "%~1"=="--install" set INSTALL_TARGET=1
if /i "%~1"=="--vs2022" set GENERATOR=Visual Studio 17 2022
if /i "%~1"=="--vs2019" set GENERATOR=Visual Studio 16 2019
if /i "%~1"=="--mingw" set GENERATOR=MinGW Makefiles
if /i "%~1"=="--ninja" set GENERATOR=Ninja
if /i "%~1"=="--sdl3-path" (
    shift
    set SDL3_PATH=%~1
)
if /i "%~1"=="--help" goto show_help
shift
goto parse_args

:end_parse

REM Auto-detect generator if not specified
if "%GENERATOR%"=="" (
    echo Detecting build environment...

    REM Check for Visual Studio 2022
    where /q cl 2>nul
    if !errorlevel! equ 0 (
        set GENERATOR=Visual Studio 17 2022
        echo Found Visual Studio 2022
        goto generator_found
    )

    REM Check for MinGW
    where /q mingw32-make 2>nul
    if !errorlevel! equ 0 (
        set GENERATOR=MinGW Makefiles
        echo Found MinGW
        goto generator_found
    )

    REM Check for Ninja
    where /q ninja 2>nul
    if !errorlevel! equ 0 (
        set GENERATOR=Ninja
        echo Found Ninja
        goto generator_found
    )

    REM Default to NMake if nothing else found
    set GENERATOR=NMake Makefiles
    echo Using NMake Makefiles as fallback
)

:generator_found

echo Build Configuration:
echo   Generator: %GENERATOR%
echo   Build Type: %BUILD_TYPE%
echo   Clean Build: %CLEAN_BUILD%
echo   Install: %INSTALL_TARGET%
if not "%SDL3_PATH%"=="" echo   SDL3 Path: %SDL3_PATH%
echo.

REM Clean build directory if requested
if %CLEAN_BUILD% equ 1 (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
    echo.
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure CMake
echo Configuring with CMake...
set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if not "%SDL3_PATH%"=="" set CMAKE_ARGS=%CMAKE_ARGS% -DSDL3_PATH="%SDL3_PATH%"

cmake -G "%GENERATOR%" %CMAKE_ARGS% ..
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed!
    goto error_exit
)

REM Build the project
echo.
echo Building project...
cmake --build . --config %BUILD_TYPE% --parallel
if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    goto error_exit
)

REM Install if requested
if %INSTALL_TARGET% equ 1 (
    echo.
    echo Installing...
    cmake --install . --config %BUILD_TYPE%
    if %errorlevel% neq 0 (
        echo ERROR: Installation failed!
        goto error_exit
    )
)

echo.
echo ========================================
echo  Build completed successfully!
echo ========================================
echo.
echo Executable location: build\bin\%BUILD_TYPE%\PlayGame.exe
echo.
echo To run the game:
echo   cd build\bin\%BUILD_TYPE%
echo   PlayGame.exe
echo.
goto end

:show_help
echo Usage: build.bat [options]
echo.
echo Options:
echo   --debug          Build in Debug mode (default: Release)
echo   --release        Build in Release mode
echo   --clean          Clean build directory before building
echo   --install        Install after building
echo   --vs2022         Use Visual Studio 2022 generator
echo   --vs2019         Use Visual Studio 2019 generator
echo   --mingw          Use MinGW Makefiles generator
echo   --ninja          Use Ninja generator
echo   --sdl3-path DIR  Specify SDL3 installation path
echo   --help           Show this help message
echo.
echo Examples:
echo   build.bat --debug --clean
echo   build.bat --release --install --vs2022
echo   build.bat --mingw --sdl3-path "C:\SDL3"
echo.
goto end

:error_exit
echo.
echo Build failed! Check the error messages above.
cd ..
exit /b 1

:end
cd ..
