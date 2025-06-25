#!/bin/bash

# Cross-platform build script for Unix/Linux/macOS
# Supports GCC, Clang, and various build systems

set -e  # Exit on any error

echo ""
echo "============================================="
echo "  PlayGame - Cross-Platform Build (Unix)"
echo "============================================="
echo ""

# Default values
BUILD_TYPE="Release"
GENERATOR=""
CLEAN_BUILD=0
INSTALL_TARGET=0
SDL3_PATH=""
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Function to show help
show_help() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --debug          Build in Debug mode (default: Release)"
    echo "  --release        Build in Release mode"
    echo "  --clean          Clean build directory before building"
    echo "  --install        Install after building"
    echo "  --make           Use Unix Makefiles generator"
    echo "  --ninja          Use Ninja generator"
    echo "  --xcode          Use Xcode generator (macOS only)"
    echo "  --sdl3-path DIR  Specify SDL3 installation path"
    echo "  --jobs N         Number of parallel jobs (default: auto-detected)"
    echo "  --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --debug --clean"
    echo "  $0 --release --install --ninja"
    echo "  $0 --xcode --sdl3-path \"/usr/local\""
    echo ""
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --install)
            INSTALL_TARGET=1
            shift
            ;;
        --make)
            GENERATOR="Unix Makefiles"
            shift
            ;;
        --ninja)
            GENERATOR="Ninja"
            shift
            ;;
        --xcode)
            GENERATOR="Xcode"
            shift
            ;;
        --sdl3-path)
            SDL3_PATH="$2"
            shift 2
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Auto-detect platform and generator if not specified
if [ -z "$GENERATOR" ]; then
    echo "Detecting build environment..."

    # Check for macOS and Xcode
    if [[ "$OSTYPE" == "darwin"* ]] && command -v xcodebuild >/dev/null 2>&1; then
        GENERATOR="Xcode"
        echo "Found Xcode on macOS"
    # Check for Ninja
    elif command -v ninja >/dev/null 2>&1; then
        GENERATOR="Ninja"
        echo "Found Ninja"
    # Default to Unix Makefiles
    else
        GENERATOR="Unix Makefiles"
        echo "Using Unix Makefiles"
    fi
fi

# Detect platform
PLATFORM="Unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
elif [[ "$OSTYPE" == "freebsd"* ]]; then
    PLATFORM="FreeBSD"
fi

echo "Build Configuration:"
echo "  Platform: $PLATFORM"
echo "  Generator: $GENERATOR"
echo "  Build Type: $BUILD_TYPE"
echo "  Clean Build: $CLEAN_BUILD"
echo "  Install: $INSTALL_TARGET"
echo "  Parallel Jobs: $JOBS"
if [ -n "$SDL3_PATH" ]; then
    echo "  SDL3 Path: $SDL3_PATH"
fi
echo ""

# Install dependencies on Linux
if [[ "$PLATFORM" == "Linux" ]]; then
    echo "Checking for required dependencies..."

    # Check if we need to install dependencies
    if ! pkg-config --exists sdl3 2>/dev/null && [ -z "$SDL3_PATH" ]; then
        echo "SDL3 not found. Please install SDL3 development libraries:"
        echo ""
        echo "Ubuntu/Debian:"
        echo "  sudo apt update"
        echo "  sudo apt install build-essential cmake libsdl3-dev"
        echo ""
        echo "Fedora/CentOS/RHEL:"
        echo "  sudo dnf install gcc-c++ cmake SDL3-devel"
        echo ""
        echo "Arch Linux:"
        echo "  sudo pacman -S base-devel cmake sdl3"
        echo ""
        echo "Or specify SDL3 path with --sdl3-path option"
        echo ""
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo ""
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
fi

# Install dependencies on macOS
if [[ "$PLATFORM" == "macOS" ]]; then
    echo "Checking for required dependencies..."

    if ! brew list sdl3 >/dev/null 2>&1 && [ -z "$SDL3_PATH" ]; then
        echo "SDL3 not found. Installing via Homebrew..."
        if command -v brew >/dev/null 2>&1; then
            echo "Installing SDL3..."
            brew install sdl3
        else
            echo "Homebrew not found. Please install SDL3 manually:"
            echo "  1. Install Homebrew: https://brew.sh"
            echo "  2. Run: brew install sdl3"
            echo "Or specify SDL3 path with --sdl3-path option"
            exit 1
        fi
    fi
fi

# Clean build directory if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo "Cleaning build directory..."
    rm -rf build
    echo ""
fi

# Create build directory
mkdir -p build
cd build

# Configure CMake
echo "Configuring with CMake..."
CMAKE_ARGS=(-DCMAKE_BUILD_TYPE="$BUILD_TYPE")

if [ -n "$SDL3_PATH" ]; then
    CMAKE_ARGS+=(-DSDL3_PATH="$SDL3_PATH")
fi

# Platform-specific configurations
if [[ "$PLATFORM" == "macOS" ]]; then
    CMAKE_ARGS+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15)
fi

cmake -G "$GENERATOR" "${CMAKE_ARGS[@]}" ..

# Build the project
echo ""
echo "Building project..."
if [[ "$GENERATOR" == "Xcode" ]]; then
    cmake --build . --config "$BUILD_TYPE" --parallel "$JOBS"
else
    cmake --build . --parallel "$JOBS"
fi

# Install if requested
if [ $INSTALL_TARGET -eq 1 ]; then
    echo ""
    echo "Installing..."
    if [[ "$GENERATOR" == "Xcode" ]]; then
        cmake --install . --config "$BUILD_TYPE"
    else
        cmake --install .
    fi
fi

echo ""
echo "========================================"
echo "  Build completed successfully!"
echo "========================================"
echo ""

# Show executable location
if [[ "$GENERATOR" == "Xcode" ]]; then
    EXECUTABLE_PATH="build/bin/$BUILD_TYPE/PlayGame"
    if [[ "$PLATFORM" == "macOS" ]]; then
        if [ -d "bin/$BUILD_TYPE/PlayGame.app" ]; then
            EXECUTABLE_PATH="build/bin/$BUILD_TYPE/PlayGame.app"
        fi
    fi
else
    EXECUTABLE_PATH="build/bin/PlayGame"
fi

echo "Executable location: $EXECUTABLE_PATH"
echo ""
echo "To run the game:"
if [[ "$EXECUTABLE_PATH" == *.app ]]; then
    echo "  open $EXECUTABLE_PATH"
else
    echo "  ./$EXECUTABLE_PATH"
fi
echo ""

cd ..
