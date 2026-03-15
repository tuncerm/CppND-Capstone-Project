# Asset Composer Format (Draft v2)

## Goal
Define a shared composition layer between raw `8x8` tiles and authoring tools.

`AssetComposer` builds reusable stamp assets:
- `CELL32` (`32x32`, made from `4x4` tiles)
- later `SPRITE` and `ANIM`

Important decision:
- Map files remain raw tile data (`128x80` tile entries, `2 bytes` each).
- Assets are editor-time convenience only (not stored as `asset_id` in map payload).

## Scope Split
- `TileMaker`:
  - owns `tiles.dat` (`8x8`, 256 tiles)
  - owns per-tile `spec` bits (`health/destruction/movement`)
- `AssetComposer`:
  - owns `assets.dat`
  - defines reusable full stamps (`CELL32`) and stamp semantics
- `MapMaker`:
  - applies selected stamp as a full `4x4` write into raw map tiles
  - for special stamp types, also updates map metadata/header fields

## IDs and Limits (v1)
- `tile_id`: `u8` (`0..255`)
- `asset_id`: `u16` (`0..65535`)
- `anim_id`: `u16` (`0..65535`)

## Binary Container (`assets.dat`)
Binary-only asset catalog.

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

### 3) Record Payloads

#### `CELL32` (`type=1`, currently implemented)
- `w_tiles u8` (=4)
- `h_tiles u8` (=4)
- `flags u16`
- `tile_refs[16]` (`u8` tile IDs)

## Full Stamp Map Integration
Map payload stays raw and final:

1. user selects `CELL32` in MapMaker
2. stamp apply writes all `16` tile entries into the target `4x4` tile area
3. map save writes only raw map header + raw tile payload

No `asset_id` dependency at runtime for map rendering.

## Special Metadata Stamps (new planned behavior)
Some assets are special authoring markers that also mutate map metadata/header.

Examples:
- player base
- enemy base
- player spawn
- enemy spawn
- flag
- factory

Planned editor behavior:
- stamp still writes its full `4x4` tile pattern
- plus metadata updater applies semantic changes (header fields/collections)
- if stamp is removed/overwritten, metadata must be recomputed or corrected

Notes:
- this is not implemented yet in `AssetComposer`/`MapMaker`
- role/tag encoding for special stamps is still open (likely in `CELL32.flags` or an extended record)

## TileMaker Import Additions (planned)
- PNG atlas import (`128x128`, `16x16` tiles, strict/remap modes)
- text specs import: `tile_id,health,destruction_mode,movement`

## Implementation Order (revised)
1. Finalize special stamp role encoding in `assets.dat`.
2. Add special-role editing UI in `AssetComposer`.
3. Add full-stamp placement in MapMaker backed by raw tile writes.
4. Add metadata/header updater triggered by special stamp placement/removal.
5. Add `SPRITE` and `ANIM`.
6. Add TileMaker PNG/spec import pipeline.
