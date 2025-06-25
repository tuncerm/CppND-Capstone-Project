#!/bin/bash

# Palette Maker Build Script
# Cross-platform build script for SDL3 Palette Maker

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Release"
CLEAN_BUILD=false
INSTALL_AFTER=false
GENERATOR=""
SDL3_PATH=""
JOBS=4

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --debug              Build in Debug mode"
    echo "  --release            Build in Release mode (default)"
    echo "  --clean              Clean build directory before building"
    echo "  --install            Install after building"
    echo "  --make               Use Unix Makefiles generator"
    echo "  --ninja              Use Ninja generator"
    echo "  --xcode              Use Xcode generator (macOS only)"
    echo "  --sdl3-path PATH     Specify SDL3 installation path"
    echo "  --jobs N             Number of parallel build jobs (default: 4)"
    echo "  --help               Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                          # Simple release build"
    echo "  $0 --debug --clean          # Clean debug build"
    echo "  $0 --ninja --install        # Build with Ninja and install"
    echo "  $0 --sdl3-path /usr/local   # Specify SDL3 location"
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
            CLEAN_BUILD=true
            shift
            ;;
        --install)
            INSTALL_AFTER=true
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
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Print build configuration
print_status "Palette Maker Build Configuration"
echo "  Build Type: $BUILD_TYPE"
echo "  Clean Build: $CLEAN_BUILD"
echo "  Install After: $INSTALL_AFTER"
echo "  Generator: ${GENERATOR:-Auto}"
echo "  SDL3 Path: ${SDL3_PATH:-Auto-detect}"
echo "  Parallel Jobs: $JOBS"
echo ""

# Check for required tools
print_status "Checking build requirements..."

if ! command -v cmake &> /dev/null; then
    print_error "CMake is required but not installed"
    echo "Install CMake from: https://cmake.org/install/"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
print_status "Found CMake version: $CMAKE_VERSION"

# Check for Ninja if specified
if [[ "$GENERATOR" == "Ninja" ]]; then
    if ! command -v ninja &> /dev/null; then
        print_error "Ninja is required but not installed"
        echo "Install Ninja: https://ninja-build.org/"
        exit 1
    fi
    print_status "Found Ninja build system"
fi

# Platform detection
PLATFORM=$(uname -s)
print_status "Detected platform: $PLATFORM"

# Set default generator based on platform
if [[ -z "$GENERATOR" ]]; then
    case $PLATFORM in
        Darwin)
            GENERATOR="Unix Makefiles"
            ;;
        Linux)
            GENERATOR="Unix Makefiles"
            ;;
        *)
            GENERATOR="Unix Makefiles"
            ;;
    esac
    print_status "Using default generator: $GENERATOR"
fi

# Create and enter build directory
BUILD_DIR="build"

if [[ "$CLEAN_BUILD" == true ]]; then
    print_status "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

if [[ ! -d "$BUILD_DIR" ]]; then
    print_status "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Prepare CMake arguments
CMAKE_ARGS=()
CMAKE_ARGS+=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE")

if [[ -n "$GENERATOR" ]]; then
    CMAKE_ARGS+=("-G" "$GENERATOR")
fi

if [[ -n "$SDL3_PATH" ]]; then
    CMAKE_ARGS+=("-DSDL3_PATH=$SDL3_PATH")
fi

# Configure
print_status "Configuring project..."
print_status "CMake command: cmake ${CMAKE_ARGS[*]} .."

if ! cmake "${CMAKE_ARGS[@]}" ..; then
    print_error "CMake configuration failed"
    exit 1
fi

print_success "Configuration completed"

# Build
print_status "Building project..."
BUILD_ARGS=("--build" "." "--config" "$BUILD_TYPE")

if [[ "$JOBS" -gt 1 ]]; then
    BUILD_ARGS+=("--parallel" "$JOBS")
fi

print_status "Build command: cmake ${BUILD_ARGS[*]}"

if ! cmake "${BUILD_ARGS[@]}"; then
    print_error "Build failed"
    exit 1
fi

print_success "Build completed successfully"

# Install if requested
if [[ "$INSTALL_AFTER" == true ]]; then
    print_status "Installing..."
    if ! cmake --build . --target install; then
        print_error "Installation failed"
        exit 1
    fi
    print_success "Installation completed"
fi

# Show results
print_success "Build Summary:"
echo "  Build Type: $BUILD_TYPE"
echo "  Generator: $GENERATOR"

# Find executable
if [[ -f "bin/PaletteMaker" ]]; then
    EXECUTABLE="bin/PaletteMaker"
elif [[ -f "bin/$BUILD_TYPE/PaletteMaker" ]]; then
    EXECUTABLE="bin/$BUILD_TYPE/PaletteMaker"
elif [[ -f "bin/PaletteMaker.exe" ]]; then
    EXECUTABLE="bin/PaletteMaker.exe"
elif [[ -f "bin/$BUILD_TYPE/PaletteMaker.exe" ]]; then
    EXECUTABLE="bin/$BUILD_TYPE/PaletteMaker.exe"
else
    EXECUTABLE="<not found>"
fi

echo "  Executable: $EXECUTABLE"

if [[ "$EXECUTABLE" != "<not found>" ]]; then
    echo ""
    print_success "To run Palette Maker:"
    echo "  cd $BUILD_DIR"
    echo "  ./$EXECUTABLE"
    echo ""
    print_status "Or with a palette file:"
    echo "  ./$EXECUTABLE my_palette.dat"
else
    print_warning "Executable not found at expected location"
    print_status "Check the bin/ directory for PaletteMaker"
fi

print_success "Build script completed successfully!"
