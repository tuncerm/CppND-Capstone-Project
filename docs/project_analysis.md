# Project Analysis Report

## Document status
This file is the source of truth for analysis outcomes and completed follow-up work.

- Baseline analysis date: `2026-03-07`
- Last status refresh: `2026-03-08` (post-implementation cleanup)
- Active tracking file: `docs/todo.md`

## Baseline summary (2026-03-07)
The original analysis identified major issues across:

- Crash prevention and bounds safety
- Path/config resolution consistency
- Shared library contract mismatches
- Performance issues in the runtime loop
- Build/test workflow gaps (root `ctest`, CI, static analysis)

## Resolution status (as of 2026-03-08)
All follow-up checklist items are closed, including:

- P0 correctness fixes (`Character`, `GameMap`, `AICentral`, startup map validation)
- P1 contract/test-stability alignment (`SDL` lifecycle policy, utility contracts)
- P2 performance cleanup (removed per-frame async; cached renderer config data)
- P3/P4 consistency cleanup (config/path usage and UI/shared cleanup tasks)
- P5 workflow improvements (root `ctest`, CI pipeline, `cppcheck`, sanitizer build option)

Validation snapshot after these updates:

- Build: `cmake --build build` passes
- Tests: `ctest --output-on-failure` passes at build root (`204/204`)

## Completed TODO summary
Follow-up TODO implementation and cleanup are complete:

1. Path determinism
   - `src/path_resolver.cpp` now resolves using executable/base-path anchored probing while preserving fallback behavior.
2. Build hygiene
   - Removed implementation include coupling by replacing `#include "tempmap.cpp"` with `src/tempmap.h` declaration + separate `src/tempmap.cpp` translation unit.
3. Runtime warning quality
   - Fixed constructor init-order and unused-parameter warnings in runtime/tool code.
   - Updated SDL init checks to SDL3 bool-style usage in shared runtime/test entry points.
4. Test portability and warning cleanup
   - Replaced C++20-only designated initialization usage in tests.
   - Reduced warning noise (`overflow`, `narrowing`, maybe-uninitialized, unused locals) without behavior changes.
   - Hardened file utility tests by isolating temp file/directory paths per run and per call.
   - Hardened config manager tests by isolating config save/load files in unique temp directories.
5. CI workflow improvements
   - Split CI builds by target family (`PlayGame`, `PaletteMaker`, `TileMaker`).
   - Isolated shared tests + analysis job.
   - Added non-blocking strict cppcheck profile for pull requests.
   - Added shared test labels and CI label filtering (`ctest -L shared`).
   - Added warnings-as-errors enforcement option (`ENABLE_WARNINGS_AS_ERRORS`) and enabled it in CI.
   - Centralized cppcheck invocation via `tools/run_cppcheck.sh` and upload strict cppcheck report artifacts in PR CI.
   - Added strict cppcheck XML summary generation for CI step summaries (`tools/summarize_cppcheck_xml.py`).
   - Added strict cppcheck severity gate support (blocking mode enforces `error=0`, `warning=0`).

## Current verification snapshot
- Build: `cmake --build build --clean-first` passes.
- Tests: `ctest --output-on-failure` and `ctest -L shared --output-on-failure` pass (`204/204`).

## Destructible map middle-layer status (2026-03-08)
Implemented in runtime:

- `GameMap` supports a 4x4 subtile middle layer per 32x32 grid cell (`16` subtiles).
- Added world-space hit mapping API (`WorldToSubtile`, `DamageAtWorldPosition`).
- Player fire action now damages one subtile in the facing direction (`F` key path).
- Renderer draws destructible cells at subtile granularity (8x8 visual blocks), so visual
  destruction aligns with subtile hitboxes.

Remaining follow-up:

- Integrate real tile pixel art rendering from tile-maker output (current runtime still uses
  color blocks, with tile-id variation only as color tinting).

## Remaining recommendations (non-blocking)
- Monitor strict cppcheck pull-request output for 1-2 cycles, then promote stable checks from non-blocking to blocking.

## Current design decisions (2026-03-09)
- Map runtime format stays raw tile-based (`128x80`, `2 bytes` per tile), with no `asset_id` dependency in map payload.
- MapMaker will use full `CELL32` stamps as an authoring workflow only (write all `16` tile entries per apply).
- Special stamp classes (`base/flag/factory/spawnpoint/etc`) are planned to update map metadata/header on placement/removal; this is documented but not implemented yet.

Tracked as active items in `docs/todo.md`.
