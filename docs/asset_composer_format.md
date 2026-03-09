# Asset Composer Format (Draft v1)

## Goal
Define a shared composition layer that sits between raw `8x8` tiles and final game content.

`AssetComposer` will build:
- `CELL32` assets for map cells (`32x32` -> `4x4` tiles)
- `SPRITE` assets for characters/objects (`NxM` tiles)
- `ANIM` assets as frame sequences over composed assets

This replaces hardcoding 16 subtiles inline in map payloads and enables reusable map/character assets.

## Scope Split
- `TileMaker`:
  - Owns `tiles.dat` (raw `8x8` tiles, 256 max in v1)
  - Owns tile-level specs (health/destruction/movement bits)
  - Adds import tools:
    - PNG atlas import (authoring convenience)
    - text specs import (authoring convenience)
- `AssetComposer`:
  - Owns composed assets database (`assets.dat`)
  - Builds map cells, sprites, and animations from tile IDs
- `MapMaker`:
  - Paints `asset_id` cells (not raw 16-subtile payloads)

## IDs and Limits (v1)
- `tile_id`: `u8` (`0..255`)
- `asset_id`: `u16` (`0..65535`)
- `anim_id`: `u16` (`0..65535`)

Use `u16` for composed assets to avoid early ceiling issues.

## Binary Container (`assets.dat`)
Binary-only runtime format.

### 1) File Header
- `magic[4] = "ASDB"`
- `version u16` (`1`)
- `asset_count u16`
- `index_offset u32`
- `flags u32` (reserved)

### 2) Index Table (`asset_count` entries)
- `asset_id u16`
- `type u8` (`1=CELL32`, `2=SPRITE`, `3=ANIM`)
- `reserved u8`
- `name[16]` (ASCII, null/space padded)
- `record_offset u32`
- `record_size u32`
- `crc32 u32`

### 3) Record Payload (by type)

#### `CELL32` (`type=1`)
- `w_tiles u8` (=4)
- `h_tiles u8` (=4)
- `flags u16` (reserved for cell-level tags)
- `tile_refs[16]` (`u8` tile IDs)

#### `SPRITE` (`type=2`)
- `w_tiles u8` (`1..16` initial target)
- `h_tiles u8` (`1..16` initial target)
- `pivot_x i8` (optional, local pivot)
- `pivot_y i8`
- `tile_refs[w_tiles*h_tiles]` (`u8`)

#### `ANIM` (`type=3`)
- `frame_count u16`
- `loop_mode u8` (`0=once`, `1=loop`, `2=pingpong`)
- `reserved u8`
- `frames[frame_count]`:
  - `asset_id u16` (must reference `SPRITE`)
  - `duration_ticks u16`
  - `flags u16` (reserved)

## Map Integration Plan
- Current: each map cell carries 16 packed subtiles.
- Target: each map cell stores one `asset_id` (`CELL32`) + map cell flags/material as needed.
- Runtime flow:
  1. read map cell `asset_id`
  2. resolve `asset_id -> CELL32`
  3. draw the resolved `4x4` tile IDs

This yields reuse, smaller map edits, and easier content iteration.

## TileMaker Import Additions
Authoring-only inputs (runtime remains binary-only):

### PNG Atlas Import
- Expected v1 atlas: `128x128` indexed image (`16x16` tiles of `8x8`)
- Palette check against `palette.dat` (16 colors)
- Modes:
  - `strict`: reject colors outside palette
  - `remap`: nearest palette entry + import report

### Text Specs Import
- Input file example: CSV/TXT lines
- `tile_id,health,destruction_mode,movement`
- Validate ranges before apply:
  - `health: 0..7`
  - `destruction_mode: 0..7`
  - `movement: 0..3`

## UI Expectations for `AssetComposer`
- Left: tile list (`0..255`)
- Center: composition canvas (`CELL32`/`SPRITE`)
- Right: asset properties + animation timeline
- Common actions: `New/Open/Save/Clone/Delete`

## Implementation Order (proposed)
1. Lock `assets.dat` bytes/structs and parser/writer.
2. Implement `CELL32` only, integrate with `MapMaker`.
3. Add `SPRITE`.
4. Add `ANIM`.
5. Add TileMaker PNG/spec imports.

