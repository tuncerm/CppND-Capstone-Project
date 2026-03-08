# Map Format

## Scope
- Map grid cell size: `32x32`.
- Each map cell contains a `4x4` subtile block.
- Each subtile is `8x8`.

This document defines the packed subtile spec used by runtime loading and future map tooling.

## Terms
- `tile`: one `8x8` art tile (palette/tile-maker output).
- `block` (prefab): one `4x4` arrangement of tiles/subtile specs used by one map cell.

## Packed Subtile Entry (16-bit)
Each subtile uses:
- `8 bits` tile ID
- `8 bits` spec flags

## Packed Cell Token
The packed map token for one `32x32` cell is:

```txt
material|e0,e1,e2,...,e15
```

- `material`: `0` walkable, `1` blocked (legacy compatibility flag)
- `e0..e15`: 16 packed `uint16` subtile entries (decimal or `0x` hex)

Example:
```txt
1|0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901,0x4901
```

### Byte layout
- `byte0` (`bits 0..7`): `tile_id` (`0..255`)
- `byte1` (`bits 8..15`): `spec`

### `spec` layout (8 bits total)
- bits `0..2` (`3 bits`): `health`
  - `0` = gone/destroyed
  - `1..7` = remaining health
- bits `3..5` (`3 bits`): `destruction_mode`
  - `000` = indestructible
  - `001` = normal
  - `010` = heavy
  - `011` = special
  - `100..111` = reserved
- bits `6..7` (`2 bits`): `movement`
  - `00` = no pass
  - `01` = pass
  - `10` = requires condition/item
  - `11` = special

## Runtime interpretation rules
- Destroyed state is **derived**, not a separate bit:
  - `health == 0` means destroyed/gone.
- `destruction_mode == 000` should ignore damage and keep health unchanged.
- Movement checks should read the `movement` bits per subtile.

## Packing helpers
```txt
spec = (health & 0x07)
     | ((destruction_mode & 0x07) << 3)
     | ((movement & 0x03) << 6)
```

```txt
health           = spec & 0x07
destruction_mode = (spec >> 3) & 0x07
movement         = (spec >> 6) & 0x03
```

## Migration note
- Runtime supports both:
  - legacy tokens (`0`/`1` grid values and legacy extended `material|t0..t15|destroyed_mask`)
  - packed tokens (`material|e0..e15`)
