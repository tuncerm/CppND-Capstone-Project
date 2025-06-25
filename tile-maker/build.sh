#!/bin/bash

echo "==============================================="
echo "Building Tile Maker for Unix/Linux/macOS..."
echo "==============================================="

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "Linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macOS"
    elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
        echo "Windows"
    else
        echo "Unknown"
    fi
}

OS=$(detect_os)
echo "Detected OS: $OS"

# Check if cmake is available
if ! command -v cmake &> /dev/null; then
    echo ""
    echo "==============================================="
    echo "ERROR: CMAKE NOT FOUND"
    echo "==============================================="
    echo "CMake is not installed or not in PATH"
    echo ""
    echo "Installation instructions:"
    case $OS in
        "Linux")
            echo "  Ubuntu/Debian: sudo apt-get install cmake"
            echo "  CentOS/RHEL: sudo yum install cmake"
            echo "  Fedora: sudo dnf install cmake"
            echo "  Arch: sudo pacman -S cmake"
            ;;
        "macOS")
            echo "  Homebrew: brew install cmake"
            echo "  MacPorts: sudo port install cmake"
            ;;
        *)
            echo "  Please install CMake for your system"
            ;;
    esac
    exit 1
fi

echo "CMake found: $(cmake --version | head -n1)"

# Check for SDL3 development libraries
echo ""
echo "Checking for SDL3 development libraries..."
SDL3_FOUND="false"

# Common SDL3 locations
SDL3_LOCATIONS=(
    "/usr/include/SDL3"
    "/usr/local/include/SDL3"
    "/opt/local/include/SDL3"
    "/usr/include/SDL3"
    "/usr/local/include"
    "/opt/homebrew/include/SDL3"  # macOS ARM Homebrew
    "/usr/local/Cellar/sdl3"      # macOS Intel Homebrew
)

for location in "${SDL3_LOCATIONS[@]}"; do
    if [ -f "$location/SDL.h" ] || [ -f "$location/SDL3/SDL.h" ]; then
        echo "  Found SDL3 headers at: $location"
        SDL3_FOUND="true"
        break
    fi
done

if [ "$SDL3_FOUND" == "false" ]; then
    echo "  Warning: SDL3 development headers not found in common locations"
    echo "  Build may fail if SDL3 is not properly installed"
fi

# Check for C compiler
echo ""
echo "Checking for C compiler..."
CC_FOUND="false"

if command -v gcc &> /dev/null; then
    echo "  Found GCC: $(gcc --version | head -n1)"
    CC_FOUND="true"
elif command -v clang &> /dev/null; then
    echo "  Found Clang: $(clang --version | head -n1)"
    CC_FOUND="true"
else
    echo "  ERROR: No suitable C compiler found"
    case $OS in
        "Linux")
            echo "  Install with: sudo apt-get install build-essential"
            ;;
        "macOS")
            echo "  Install Xcode Command Line Tools: xcode-select --install"
            ;;
    esac
    exit 1
fi

# Create build directory
echo ""
echo "Setting up build directory..."
mkdir -p build
cd build

# Configure with CMake
echo ""
echo "==============================================="
echo "CONFIGURING CMAKE..."
echo "==============================================="
cmake .. -DCMAKE_BUILD_TYPE=Release

# Check if CMake configuration succeeded
if [ $? -ne 0 ]; then
    echo ""
    echo "==============================================="
    echo "CMAKE CONFIGURATION FAILED!"
    echo "==============================================="
    echo "Make sure you have:"
    echo "  1. SDL3 development libraries installed"
    echo "  2. A C compiler (gcc or clang) installed"
    echo "  3. CMake installed"
    echo ""

    case $OS in
        "Linux")
            echo "Ubuntu/Debian installation:"
            echo "  sudo apt-get install build-essential cmake"
            echo "  # Note: SDL3 may need manual installation"
            echo ""
            echo "CentOS/RHEL installation:"
            echo "  sudo yum install gcc cmake"
            echo "  # Note: SDL3 may need manual installation"
            echo ""
            echo "Fedora installation:"
            echo "  sudo dnf install gcc cmake"
            echo "  # Note: SDL3 may need manual installation"
            ;;
        "macOS")
            echo "macOS installation:"
            echo "  brew install cmake"
            echo "  # Note: SDL3 may need manual installation or:"
            echo "  # brew install sdl3 (if available)"
            ;;
    esac
    echo ""
    echo "SDL3 Installation Notes:"
    echo "  SDL3 is still in development and may not be available"
    echo "  in standard package repositories. You may need to:"
    echo "  1. Build SDL3 from source"
    echo "  2. Use a development package manager"
    echo "  3. Install pre-built binaries"
    echo ""
    exit 1
fi

# Build the project
echo ""
echo "==============================================="
echo "BUILDING PROJECT..."
echo "==============================================="

# Use parallel builds if available
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "1")
echo "Using $PARALLEL_JOBS parallel jobs"

cmake --build . --config Release -- -j$PARALLEL_JOBS

# Check if build succeeded
if [ $? -ne 0 ]; then
    echo ""
    echo "==============================================="
    echo "BUILD FAILED!"
    echo "==============================================="
    echo "Check the error messages above for details."
    echo ""
    echo "Common issues:"
    echo "1. Missing SDL3 development libraries"
    echo "2. Incompatible compiler/library versions"
    echo "3. Missing system dependencies"
    echo ""
    echo "For help, check the build output above and ensure"
    echo "all prerequisites are properly installed."
    echo ""
    exit 1
fi

echo ""
echo "==============================================="
echo "BUILD COMPLETED SUCCESSFULLY!"
echo "==============================================="

# Check for executable
EXE_PATH="bin/TileMaker"
if [ -f "$EXE_PATH" ]; then
    echo "Executable: $EXE_PATH"

    # Make the executable, well, executable
    chmod +x "$EXE_PATH"
    echo "Permissions set"

    echo ""
    echo "To run the application:"
    echo "  cd build/bin"
    echo "  ./TileMaker"
    echo ""

    # Optionally run the application if requested
    if [ "$1" = "--run" ]; then
        echo "Running Tile Maker..."
        cd bin
        ./TileMaker
    fi
else
    echo "Warning: Executable not found at expected location: $EXE_PATH"
    echo "Check build output for actual location"

    # Try to find the executable
    FOUND_EXE=$(find . -name "TileMaker" -type f 2>/dev/null | head -n1)
    if [ -n "$FOUND_EXE" ]; then
        echo "Found executable at: $FOUND_EXE"
        chmod +x "$FOUND_EXE"
    fi
fi

echo ""
echo "Build completed for $OS using $(cmake --version | head -n1)"
echo ""
