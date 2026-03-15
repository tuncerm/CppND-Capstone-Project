# Map DB Format (Binary Only)

## Goal
Define a binary-only map storage format for multiple maps with metadata.

Constraints:
- No JSON/text metadata.
- Map size max: `32x20`.
- Tile system: `32x32` cells, each cell stores `16` packed subtiles (`uint16` each).
- Keep a map catalog concept like: `NO | TITLE(16) | METADATA`.

## Single Map v1 Constraints (Current Focus)
- Player spawns: exactly `2`.
- Enemy spawns: exactly `3`.
- Player home base: max `1`.
- Enemy base: max `1` (optional).
- Enemy base can produce extra enemies outside map enemy count.
- Structure definition:
  - upper-left map coordinate: `x`, `y` (cell grid, max `32x20`)
  - size: `w`, `h`
  - tile-source origin: `tile_x`, `tile_y` on a `16x16` tile grid.

### Single-file Layout (Current)
- One file (`game.map`) contains both metadata and map payload.
- Bytes `0..63`: fixed metadata header (`64` bytes).
- Bytes `64..`: map cell payload (current text token payload for compatibility).

## Option A: Fixed Record (Simple)
Binary file layout:

1. File header
- `magic[4] = "MAPD"`
- `version u16` (`1`)
- `record_count u16`
- `record_size u32`

2. Records (fixed-size)
- `map_no u16`
- `title[16]` (ASCII, null/space padded)
- `metadata` (fixed struct)
- `cells[32*20]` (fixed maximum map area)

Pros:
- Very easy to implement.
- O(1) random access by record index.

Cons:
- Wastes space for smaller maps.
- Harder to evolve when metadata grows.

## Recommended Option B: Indexed Chunked Records (Better)
Use a binary container with an index table and variable-size map payloads.

### File layout
1. Container header
- `magic[4] = "MAPD"`
- `version u16` (`1`)
- `map_count u16`
- `index_offset u32`
- `flags u32` (reserved)

2. Map index entries (`map_count` items)
- `map_no u16`
- `title[16]`
- `map_flags u16` (reserved)
- `record_offset u32`
- `record_size u32`
- `crc32 u32` (record payload checksum)

3. Map record payload at `record_offset`
- `record_magic[4] = "MAPR"`
- `record_version u16`
- `width u8` (`<=32`)
- `height u8` (`<=20`)
- `chunk_count u16`
- chunks...

4. Chunk format (TLV-style)
- `chunk_id u16`
- `chunk_size u32`
- `chunk_payload[chunk_size]`

### Standard chunks (v1)
- `0x0001` `META`:
  - mode (`u8`)
  - player spawns
  - home base rect + type
  - enemy bases/spawns
  - enemy counts/factory flags
- `0x0002` `GRID`:
  - `width*height` cells
  - each cell:
    - `material u8`
    - `reserved u8`
    - `subtiles[16] u16`
- `0x0003` `TAGS` (optional):
  - arbitrary tags/ids for filtering

Pros:
- Binary-only and future-proof.
- Backward/forward compatible (unknown chunks can be skipped).
- No wasted full `32x20` storage for smaller maps.
- Safer corruption handling with per-record CRC.

Cons:
- Slightly more code than fixed record format.

## Metadata Fields (v1)
Suggested `META` chunk fields:
- `mode u8` (`0=normal`, `1=battle`)
- `player_count u8` (1..2)
- `player_spawn[2]` (`x u8, y u8`)
- `home_base_count u8`
- `home_base[]` (`x,y,w,h,type_id`)
- `enemy_base_count u8`
- `enemy_base[]` (`x,y,w,h,type_id`)
- `enemy_spawn_count u8`
- `enemy_spawn[]` (`x,y`)
- `initial_enemy_count u8`
- `max_alive_enemies u8`
- `factory_enabled u8`
- `factory_spawn_interval_sec u16`
- `factory_flags u16` (reserved)

### Metadata Source Rule (authoring)
Metadata can be edited directly, but MapMaker should also support deriving/updating metadata
from special full-stamp assets placed on the map (base/flag/factory/spawnpoint classes).

Recommended rule for consistency:
- placement/removal of a special stamp updates in-memory `META`
- before save, run a metadata validation/rebuild pass from the current map state

## Why Option B is Better
Option B preserves your `NO|TITLE|METADATA` idea but avoids locking the project into a rigid struct.
It gives room for:
- New gameplay metadata without breaking old maps.
- Multiple modes/types in one DB.
- Incremental tool evolution (MapMaker can add chunks later).

## Suggested Next Step
Implement Option B as `maps.db` while keeping current `src/game.map` export/import during transition, and
add special-stamp metadata synchronization in MapMaker.
