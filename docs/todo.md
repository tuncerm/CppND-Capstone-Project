# Active TODO

## Current items
- [ ] Finalize MapMaker modal flow QA: startup/menu/open/save/new/quit confirm paths and edge cases.
- [ ] Add real map-path selection UI (replace current placeholder list) for open/save.
- [ ] Review strict cppcheck pull-request reports over 1-2 cycles and set `CPPCHECK_STRICT_BLOCKING=true`.
- [ ] Add damage-type routing in gameplay (`normal/heavy/special`) to use all `destruction_mode` values.
- [ ] Integrate tile-maker pixel data into PlayGame renderer (replace color-tinted subtile placeholders).

## Asset Composer roadmap
- [ ] Freeze `assets.dat` v1 schema update for special stamp roles/tags.
- [x] Create `AssetComposer` tool skeleton with binary load/save and basic dialogs.
- [x] Implement `CELL32` authoring (`4x4` of `8x8` tile IDs) and persistence in `assets.dat`.
- [ ] Add `CELL32` role/tag editing for special metadata stamps (`base/flag/factory/spawnpoint/etc`).
- [ ] Add validation rules for special assets (counts/uniqueness/required fields).
- [ ] Add MapMaker full-stamp placement flow (always write full `4x4`, no partial stamping).
- [ ] Add MapMaker metadata/header updater triggered by special stamp placement/removal.
- [ ] Add metadata rebuild/validation pass before map save to keep header consistent.
- [ ] Add `SPRITE` and `ANIM` record authoring to `AssetComposer`.

## Tile import pipeline
- [ ] Add TileMaker PNG atlas import (`128x128`, `16x16` tiles, strict/remap modes).
- [ ] Add TileMaker text specs import (`tile_id,health,destruction_mode,movement`) with validation/reporting.

## Notes
- Full analysis context: `docs/project_analysis.md`
