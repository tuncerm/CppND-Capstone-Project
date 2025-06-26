# Constants Guide

This document outlines the new constants introduced to eliminate magic numbers from the codebase. These constants are organized into separate header files for each major component of the project.

## `src/constants.h`

This file contains constants related to the main game application.

| Constant | Value | Description |
|---|---|---|
| `GRID_SIZE` | 32 | The size of each grid cell in pixels. |
| `GRID_WIDTH` | 32 | The width of the game grid in cells. |
| `GRID_HEIGHT` | 20 | The height of the game grid in cells. |
| `TARGET_FPS` | 60 | The target frames per second for the game. |
| `MS_PER_FRAME` | 16 | The number of milliseconds per frame, calculated from `TARGET_FPS`. |
| `EYE_OFFSET_X` | 8 | The X offset for character eyes. |
| `EYE_OFFSET_Y` | 8 | The Y offset for character eyes. |
| `EYE_WIDTH` | 6 | The width of character eyes. |
| `EYE_HEIGHT` | 8 | The height of character eyes. |
| `EYE_SPACING` | 10 | The spacing between character eyes. |
| `MOUTH_OFFSET_X` | 8 | The X offset for the character's mouth. |
| `MOUTH_OFFSET_Y` | 24 | The Y offset for the character's mouth. |
| `MOUTH_WIDTH` | 16 | The width of the character's mouth. |
| `MOUTH_HEIGHT` | 6 | The height of the character's mouth. |
| `WINDOW_WIDTH` | 800 | The width of the main game window. |
| `WINDOW_HEIGHT` | 600 | The height of the main game window. |

---

## `tile-maker/constants.h`

This file contains constants for the `tile-maker` utility.

| Constant | Value | Description |
|---|---|---|
| `WINDOW_WIDTH` | 900 | The width of the tile-maker window. |
| `WINDOW_HEIGHT` | 600 | The height of the tile-maker window. |
| `PALETTE_BAR_HEIGHT` | 40 | The height of the palette bar in the UI. |
| `BUTTON_WIDTH` | 80 | The width of UI buttons. |
| `BUTTON_HEIGHT` | 30 | The height of UI buttons. |
| `PALETTE_SWATCH_SIZE` | 24 | The size of each color swatch in the palette. |
| `TARGET_FPS` | 60 | The target frames per second for the UI. |
| `FRAME_DELAY_MS` | 16 | The delay between frames in milliseconds. |
| `TILE_SHEET_POS_X` | 10 | The X position of the tile sheet. |
| `TILE_SHEET_POS_Y` | 50 | The Y position of the tile sheet. |
| `PIXEL_EDITOR_POS_X` | 280 | The X position of the pixel editor. |
| `PIXEL_EDITOR_POS_Y` | 50 | The Y position of the pixel editor. |
| `CLEAR_COLOR_R` | 32 | The red component of the background color. |
| `CLEAR_COLOR_G` | 32 | The green component of the background color. |
| `CLEAR_COLOR_B` | 32 | The blue component of the background color. |
| `CLEAR_COLOR_A` | 255 | The alpha component of the background color. |
| `PALETTE_SELECTION_OFFSET` | 10 | The offset for palette selection UI actions. |
| `MOUSE_BUTTON_LIMIT` | 8 | The maximum number of mouse buttons to track. |

---

## `palette-maker/constants.h`

This file contains constants for the `palette-maker` utility.

| Constant | Value | Description |
|---|---|---|
| `TARGET_FPS` | 60 | The target frames per second for the UI. |
| `FRAME_DELAY_MS` | 16 | The delay between frames in milliseconds. |

---

## `shared/constants.h`

This file contains constants for the shared components library.

| Constant | Value | Description |
|---|---|---|
| `FILE_BUFFER_SIZE` | 4096 | The buffer size for file operations. |
| `VERSION_STRING_SIZE` | 32 | The buffer size for the version string. |
| `MAX_TEXT_LEN` | 32 | The maximum length for text strings. |
| `MAX_DIGITS` | 16 | The maximum number of digits for numeric strings. |
| `INFO_BUFFER_SIZE` | 64 | The buffer size for general information strings. |
| `PERF_INFO_BUFFER_SIZE` | 128 | The buffer size for performance information strings. |
| `STRESS_TEXT_BUFFER_SIZE` | 32 | The buffer size for stress test strings. |
| `MIN_ASCII` | 32 | The minimum valid ASCII value for filenames. |
| `PALETTE_FILE_SIZE` | 64 | The expected size of a palette file in bytes. |
| `PALETTE_RGB_SIZE` | 48 | The size of an RGB palette in bytes. |
| `CHAR_WIDTH` | 6 | The width of a character in the fallback font. |
| `MAX_CHARS_PER_LINE` | 20 | The maximum number of characters per line for the fallback font. |
| `DEFAULT_TEXT_COLOR_R` | 255 | The red component of the default text color. |
| `DEFAULT_TEXT_COLOR_G` | 255 | The green component of the default text color. |
| `DEFAULT_TEXT_COLOR_B` | 255 | The blue component of the default text color. |
| `DEFAULT_TEXT_COLOR_A` | 255 | The alpha component of the default text color. |
| `TARGET_FPS` | 60 | The target frames per second for examples. |
| `FRAME_DELAY_MS` | 16 | The delay between frames in milliseconds for examples. |
