# Tile Maker - SDL3 Tile Editor

A powerful 8x8 pixel tile editor built with SDL3, designed for creating retro-style game graphics with 4-bit palette indexing.

## Features

- **64-Tile Editor**: Create and edit a complete set of 64 tiles (8×8 grid layout)
- **8×8 Pixel Tiles**: Each tile is exactly 8×8 pixels for authentic retro aesthetics
- **4-Bit Palette Indexing**: Each pixel stores a 4-bit index (0-15) into a 16-color RGBA palette
- **Comprehensive UI**: Intuitive interface with tile sheet, pixel editor, and palette bar
- **File I/O**: Load/save tiles in compact binary format (tiles.dat)
- **Palette Integration**: Uses palettes created by the companion Palette Maker application
- **Real-time Editing**: Paint, pick colors, and see changes instantly
- **Keyboard Shortcuts**: Efficient workflow with hotkeys for all major functions

## Window Layout

The application features a clean, organized layout (~900×600 pixels):

- **Tile Sheet Panel** (Left, 256×256px): Visual atlas of all 64 tiles at 4× magnification
- **Pixel Editor** (Center, 256×256px): Magnified view of selected tile (32px per pixel)
- **Palette Bar** (Bottom): 16-color palette with current selection indicator
- **Control Buttons**: Save, Load, New, and Quit buttons with status display

## File Formats

### tiles.dat
- **Size**: 2,048 bytes (64 tiles × 32 bytes each)
- **Format**: 4-bit packed pixels (2 pixels per byte, high nibble first)
- **Structure**: Each tile is 8×8 pixels = 64 pixels = 32 bytes

### palette.dat
- **Size**: 64 bytes (16 colors × 4 bytes RGBA each)
- **Format**: Standard RGBA format compatible with Palette Maker
- **Usage**: Referenced by 4-bit indices in tile data

## Controls

### Mouse Controls
- **Left Click**: Select tile in tile sheet, paint pixel in editor
- **Right Click**: Pick color from pixel in editor
- **Double-Click**: Open tile in pixel editor
- **Drag**: Paint continuously while holding left mouse button

### Keyboard Shortcuts
- **S**: Save tiles to tiles.dat
- **L**: Load tiles from tiles.dat
- **G**: Toggle pixel grid overlay
- **Ctrl+N**: Clear all tiles (with confirmation)
- **← / →**: Navigate tile selection horizontally
- **↑ / ↓**: Navigate tile selection vertically
- **ESC**: Quit application (prompts to save if modified)

### UI Interactions
- **Palette Swatches**: Click to select paint color
- **Save Button**: Save current tile set
- **Load Button**: Load existing tile set
- **New Button**: Clear all tiles
- **Quit Button**: Exit application

## Building

### Prerequisites

**All Platforms:**
- CMake 3.21 or higher
- SDL3 development libraries
- C11-compatible compiler

**Windows:**
- Visual Studio 2022 with C++ support, OR
- MinGW-w64 with GCC

**Linux:**
- GCC or Clang
- Development packages: `build-essential cmake libsdl2-dev`

**macOS:**
- Xcode Command Line Tools
- Homebrew: `brew install cmake sdl2`

### Build Instructions

**Windows (Visual Studio):**
```batch
# Run the build script
build.bat

# Or manually:
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

**Linux/macOS:**
```bash
# Make build script executable and run
chmod +x build.sh
./build.sh

# Or manually:
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Running the Application

**Windows:**
```batch
cd build\bin\Release
TileMaker.exe
```

**Linux/macOS:**
```bash
cd build/bin
./TileMaker
```

## Usage Guide

### Getting Started

1. **Launch the Application**: Run TileMaker from the build directory
2. **Load a Palette**: The app will try to load `palette.dat` (created by Palette Maker)
3. **Start Creating**: Click on tiles in the tile sheet to select them
4. **Edit Pixels**: Double-click a tile to edit it in the pixel editor
5. **Paint**: Use left mouse to paint with the selected color
6. **Pick Colors**: Use right mouse to pick colors from existing pixels
7. **Save Your Work**: Press 'S' or click Save to save your tiles

### Workflow Tips

- **Palette First**: Create your palette using the Palette Maker before starting
- **Grid Toggle**: Use 'G' to toggle the pixel grid for precise editing
- **Keyboard Navigation**: Use arrow keys to quickly navigate between tiles
- **Save Often**: The dirty indicator (*) shows when you have unsaved changes
- **Color Picking**: Right-click to quickly pick colors from existing artwork

### File Management

- **Auto-Load**: On startup, the app attempts to load `tiles.dat` and `palette.dat`
- **Save Format**: Tiles are saved in a compact binary format (2,048 bytes)
- **Backup**: Consider backing up your `.dat` files regularly
- **Portability**: Tile and palette files are cross-platform compatible

## Technical Details

### Architecture

The application is structured with clean separation of concerns:

- **main.c**: Application lifecycle and event loop
- **palette_io.c**: Palette loading/saving and color management
- **tiles_io.c**: Tile data storage and manipulation
- **tile_sheet.c**: Tile atlas rendering and selection
- **pixel_editor.c**: Magnified tile editing interface
- **ui.c**: User interface components and interactions

### Memory Management

- **Efficient Storage**: 4-bit pixels packed 2 per byte
- **Texture Caching**: Tile textures regenerated only when modified
- **Dirty Tracking**: Minimal updates for optimal performance
- **Resource Cleanup**: Proper SDL resource management

### Cross-Platform Compatibility

- **SDL3**: Modern, cross-platform multimedia library
- **CMake**: Universal build system
- **C11**: Standard C for maximum compatibility
- **Tested Platforms**: Windows, Linux, macOS

## Integration with Palette Maker

This application is designed to work seamlessly with the Palette Maker:

1. **Create Palette**: Use Palette Maker to design your 16-color palette
2. **Save Palette**: Export as `palette.dat`
3. **Load in Tile Maker**: Palette automatically loads on startup
4. **Edit Tiles**: Create tiles using your custom palette
5. **Export**: Save tiles as `tiles.dat` for use in games

## Troubleshooting

### Common Issues

**Build Errors:**
- Ensure SDL3 development libraries are installed
- Check CMake version (3.21+ required)
- Verify compiler installation

**Runtime Issues:**
- Place `palette.dat` in the same directory as the executable
- Ensure sufficient permissions for file I/O
- Check console output for detailed error messages

**Performance:**
- The application targets 60 FPS
- Large numbers of tile modifications may cause brief slowdowns
- Consider saving work periodically during intensive editing

### Getting Help

For issues, questions, or contributions:
1. Check the console output for error messages
2. Verify file permissions and directory structure
3. Ensure palette.dat is present and valid
4. Try the default palette if custom palette fails to load

## License

This project is part of the CppND Capstone Project and follows the project's licensing terms.

## Version History

- **v1.0.0**: Initial release with complete tile editing functionality
  - 64-tile editor with 8×8 pixel resolution
  - 4-bit palette indexing system
  - Comprehensive UI with real-time editing
  - Cross-platform SDL3 implementation
  - File I/O for tiles.dat and palette.dat

---

*Tile Maker - Creating retro graphics, one pixel at a time.*
