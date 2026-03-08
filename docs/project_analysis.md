# Project Analysis Report (2026-03-07)

## Document status
This file is now a cleaned historical baseline.

- Baseline analysis date: `2026-03-07`
- Last status refresh: `2026-03-08`
- Current detailed tracking source: `docs/todo_from_project_analysis.md`

## Baseline summary (2026-03-07)
The original analysis identified major issues across:

- Crash prevention and bounds safety
- Path/config resolution consistency
- Shared library contract mismatches
- Performance issues in the runtime loop
- Build/test workflow gaps (root `ctest`, CI, static analysis)

## Resolution status (as of 2026-03-08)
All checklist items from the follow-up TODO document are now closed, including:

- P0 correctness fixes (`Character`, `GameMap`, `AICentral`, startup map validation)
- P1 contract/test-stability alignment (`SDL` lifecycle policy, utility contracts)
- P2 performance cleanup (removed per-frame async; cached renderer config data)
- P3/P4 consistency cleanup (config/path usage and UI/shared cleanup tasks)
- P5 workflow improvements (root `ctest`, CI pipeline, `cppcheck`, sanitizer build option)

Validation snapshot after these updates:

- Build: `cmake --build build` passes
- Tests: `ctest --output-on-failure` passes at build root (`204/204`)

## Remaining recommendations (non-blocking)
- Keep monitoring warning drift as code evolves.
- Review strict cppcheck PR output periodically and promote stable rules to blocking checks.

## Follow-up Analysis (2026-03-08)

### Current verification snapshot
- Build: `cmake --build build --clean-first` passes.
- Tests: `ctest --output-on-failure` passes at build root (`204/204`).
- Working tree note: `palette.dat` remains locally modified (data file only).

### Findings closure update

Follow-up findings from the same date are now implemented:

1. Deterministic path resolution was implemented in `src/path_resolver.cpp` using executable/base-path anchored probing with retained fallback behavior.
2. `tempmap.cpp` include coupling was removed by introducing `src/tempmap.h` and compiling `src/tempmap.cpp` as its own translation unit.
3. Production warning fixes were applied in `src/renderer.cpp`, `src/controller.cpp`, and `tile-maker/tile_sheet.c`.
4. SDL init checks were updated to SDL3 bool style in `shared/sdl_framework/sdl_context.c` and `shared/tests/test_main.cpp`.
5. Test warning cleanup and C++17 compatibility updates were implemented across:
   - `shared/tests/unit/test_sdl_context.cpp`
   - `shared/tests/unit/test_palette_manager.cpp`
   - `shared/tests/unit/test_double_click.cpp`
6. CI was split by target family in `.github/workflows/ci.yml`, with:
   - target-matrix build jobs (`PlayGame`, `PaletteMaker`, `TileMaker`)
   - dedicated shared tests/analysis job
   - optional non-blocking strict cppcheck profile for pull requests

### Recommended next focus
- Watch strict cppcheck pull-request output for 1-2 cycles, then promote stable checks from non-blocking to blocking where signal quality is high.
- Consider adding targeted label/regex filters in `ctest` if future test suites diverge beyond shared components.
