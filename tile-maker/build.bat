@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo Building Tile Maker for Windows...
echo ===============================================

:: Create build directory
if not exist build mkdir build
cd build

:: Function to detect available toolchains
echo Detecting available toolchains...

:: Check for Visual Studio 2022
set "VS2022_FOUND=false"
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VS2022_FOUND=true"
    set "VS2022_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VS2022_FOUND=true"
    set "VS2022_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VS2022_FOUND=true"
    set "VS2022_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
)

:: Check for MinGW
set "MINGW_FOUND=false"
set "MINGW_PATH="
if exist "C:\msys64\mingw64\bin\gcc.exe" (
    set "MINGW_FOUND=true"
    set "MINGW_PATH=C:\msys64\mingw64\bin"
) else if exist "C:\MinGW\bin\gcc.exe" (
    set "MINGW_FOUND=true"
    set "MINGW_PATH=C:\MinGW\bin"
)

:: Check for SDL3 installations
echo Checking SDL3 installations...
set "SDL3_MSVC_FOUND=false"
set "SDL3_MINGW_FOUND=false"

:: Check for MinGW SDL3
if exist "C:\msys64\mingw64\include\SDL3\SDL.h" (
    set "SDL3_MINGW_FOUND=true"
    echo   - Found MinGW SDL3 at C:\msys64\mingw64\
)
if exist "C:\msys64\mingw64\include\SDL.h" (
    set "SDL3_MINGW_FOUND=true"
    echo   - Found MinGW SDL3 at C:\msys64\mingw64\
)

:: Check for MSVC SDL3 (vcpkg, manual install)
if exist "C:\vcpkg\installed\x64-windows\include\SDL3\SDL.h" (
    set "SDL3_MSVC_FOUND=true"
    echo   - Found MSVC SDL3 at C:\vcpkg\installed\x64-windows\
)
if exist "C:\SDL3\include\SDL3\SDL.h" (
    set "SDL3_MSVC_FOUND=true"
    echo   - Found MSVC SDL3 at C:\SDL3\
)
if exist "C:\Program Files\SDL3\include\SDL3\SDL.h" (
    set "SDL3_MSVC_FOUND=true"
    echo   - Found MSVC SDL3 at C:\Program Files\SDL3\
)

:: Display detected configurations
echo.
echo ===============================================
echo DETECTED CONFIGURATIONS:
echo ===============================================
echo Visual Studio 2022: !VS2022_FOUND!
echo MinGW/GCC:          !MINGW_FOUND!
echo SDL3 (MSVC):        !SDL3_MSVC_FOUND!
echo SDL3 (MinGW):       !SDL3_MINGW_FOUND!
echo.

:: Determine best build strategy
set "BUILD_STRATEGY="
set "CMAKE_GENERATOR="
set "CMAKE_ARGS="

if "!VS2022_FOUND!"=="true" if "!SDL3_MSVC_FOUND!"=="true" (
    echo Strategy: MSVC + MSVC SDL3 ^(Recommended^)
    set "BUILD_STRATEGY=MSVC"
    set "CMAKE_GENERATOR=Visual Studio 17 2022"
    set "CMAKE_ARGS=-A x64"
) else if "!MINGW_FOUND!"=="true" if "!SDL3_MINGW_FOUND!"=="true" (
    echo Strategy: MinGW + MinGW SDL3 ^(Compatible^)
    set "BUILD_STRATEGY=MINGW"
    set "CMAKE_GENERATOR=MinGW Makefiles"
    set "PATH=!MINGW_PATH!;!PATH!"
    set "CMAKE_ARGS=-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++"
) else if "!VS2022_FOUND!"=="true" if "!SDL3_MINGW_FOUND!"=="true" (
    echo.
    echo ===============================================
    echo TOOLCHAIN MISMATCH DETECTED!
    echo ===============================================
    echo Problem: MSVC compiler found but only MinGW SDL3 available
    echo This will cause compilation errors due to incompatible headers
    echo.
    echo SOLUTIONS:
    echo 1. Install MSVC-compatible SDL3:
    echo    - Use vcpkg: vcpkg install sdl3:x64-windows
    echo    - Download SDL3 development libraries from SDL website
    echo.
    echo 2. Use MinGW build instead:
    echo    - Install MinGW: pacman -S mingw-w64-x86_64-gcc
    echo    - Then run this script again
    echo.
    echo 3. Force MinGW build ^(experimental^):
    echo    - Set FORCE_MINGW=1 and run again
    echo.

    if "!FORCE_MINGW!"=="1" (
        if "!MINGW_FOUND!"=="true" (
            echo FORCING MinGW build...
            set "BUILD_STRATEGY=MINGW"
            set "CMAKE_GENERATOR=MinGW Makefiles"
            set "PATH=!MINGW_PATH!;!PATH!"
            set "CMAKE_ARGS=-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++"
        ) else (
            echo ERROR: MinGW not found! Cannot force MinGW build.
            exit /b 1
        )
    ) else (
        exit /b 1
    )
) else if "!MINGW_FOUND!"=="true" if "!SDL3_MSVC_FOUND!"=="true" (
    echo Strategy: MinGW + MSVC SDL3 ^(May work^)
    set "BUILD_STRATEGY=MINGW"
    set "CMAKE_GENERATOR=MinGW Makefiles"
    set "PATH=!MINGW_PATH!;!PATH!"
    set "CMAKE_ARGS=-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++"
) else (
    echo.
    echo ===============================================
    echo NO COMPATIBLE CONFIGURATION FOUND!
    echo ===============================================
    echo Please install one of the following:
    echo.
    echo Option 1: Visual Studio 2022 + MSVC SDL3
    echo   - Download Visual Studio 2022 Community
    echo   - Install SDL3 via vcpkg: vcpkg install sdl3:x64-windows
    echo.
    echo Option 2: MinGW + MinGW SDL3
    echo   - Install MSYS2: https://www.msys2.org/
    echo   - Install packages: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL3
    echo.
    exit /b 1
)

echo.
echo ===============================================
echo BUILDING WITH: !BUILD_STRATEGY!
echo Generator: !CMAKE_GENERATOR!
echo Args: !CMAKE_ARGS!
echo ===============================================
echo.

:: Configure with CMake
echo Configuring CMake...
if "!CMAKE_ARGS!"=="" (
    cmake .. -G "!CMAKE_GENERATOR!"
) else (
    cmake .. -G "!CMAKE_GENERATOR!" !CMAKE_ARGS!
)

:: Check if CMake configuration succeeded
if %ERRORLEVEL% neq 0 (
    echo.
    echo ===============================================
    echo CMAKE CONFIGURATION FAILED!
    echo ===============================================
    echo This could indicate:
    echo 1. Missing SDL3 development libraries
    echo 2. Compiler/library mismatch
    echo 3. Missing build tools
    echo.
    echo Current strategy was: !BUILD_STRATEGY!
    echo.
    echo Try the following:
    echo 1. Check the error messages above
    echo 2. Verify SDL3 installation
    echo 3. Try a different toolchain combination
    echo.
    exit /b 1
)

:: Build the project
echo.
echo Building project...
if "!BUILD_STRATEGY!"=="MINGW" (
    cmake --build . --config Release -- -j4
) else (
    cmake --build . --config Release
)

:: Check if build succeeded
if %ERRORLEVEL% neq 0 (
    echo.
    echo ===============================================
    echo BUILD FAILED!
    echo ===============================================
    echo Check the error messages above for details.
    echo.
    echo Common issues:
    echo 1. Compiler/library mismatch
    echo 2. Missing dependencies
    echo 3. Incompatible SDL3 headers
    echo.
    exit /b 1
)

echo.
echo ===============================================
echo BUILD COMPLETED SUCCESSFULLY!
echo ===============================================

:: Determine executable location
if "!BUILD_STRATEGY!"=="MINGW" (
    set "EXE_PATH=bin\TileMaker.exe"
) else (
    set "EXE_PATH=bin\Release\TileMaker.exe"
)

if exist "!EXE_PATH!" (
    echo Executable: !EXE_PATH!
    echo.
    echo To run the application:
    echo   cd build\!EXE_PATH:TileMaker.exe=!
    echo   TileMaker.exe
) else (
    echo Warning: Executable not found at expected location
    echo Check build output for actual location
)

echo.
echo Build strategy used: !BUILD_STRATEGY!
echo.
