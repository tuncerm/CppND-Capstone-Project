# Constants Guide

Reference for where constants live and which files should be treated as source of truth.

## Build-related switches
- Root test discovery: enabled in `CMakeLists.txt` via `include(CTest)`.
- Optional sanitizers: `-DENABLE_SANITIZERS=ON` for non-Windows GCC/Clang debug builds.

## Main game constants
File: `src/constants.h`

Key groups:
- Grid/layout: `GRID_SIZE`, `GRID_WIDTH`, `GRID_HEIGHT`
- Frame timing: `TARGET_FPS`, `MS_PER_FRAME`
- Character rendering: eye/mouth offsets and dimensions
- Window dimensions: `WINDOW_WIDTH`, `WINDOW_HEIGHT`

## Tile maker constants
File: `tile-maker/constants.h`

Key groups:
- Window/layout: `WINDOW_WIDTH`, `WINDOW_HEIGHT`, `PALETTE_BAR_HEIGHT`
- Control sizes: `BUTTON_WIDTH`, `BUTTON_HEIGHT`, `PALETTE_SWATCH_SIZE`
- Frame timing: `TARGET_FPS`, `FRAME_DELAY_MS`
- Editor positioning/input: `TILE_SHEET_POS_*`, `PIXEL_EDITOR_POS_*`, `MOUSE_BUTTON_LIMIT`

## Palette maker constants
Palette maker does not use a dedicated `constants.h` file.

Current sources:
- Runtime/config defaults: `palette-maker/config.c`, `palette-maker/config.h`
- UI layout/color macros: `palette-maker/ui.h`

Key defaults in config:
- Window: `window_width = 800`, `window_height = 600`
- Frame timing: `target_fps = 60`, `frame_delay_ms = 16`
- Palette: `color_count = 16`, `default_file = "palette.dat"`

## Shared library constants
File: `shared/constants.h`

Key groups:
- Buffer sizes and text limits
- Palette file validation constants
- Text rendering defaults
- Shared frame timing defaults
