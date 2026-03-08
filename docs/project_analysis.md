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
- Keep reducing compile warnings (constructor init-order warnings and test warning noise).
- Consider splitting CI jobs by target family (`PlayGame`, `PaletteMaker`, `TileMaker`, `shared`).
- Add optional stricter linting profile for PRs once warning volume is reduced.

## Follow-up Analysis (2026-03-08)

### Current verification snapshot
- Build: `cmake --build build --clean-first` passes.
- Tests: `ctest --output-on-failure` passes at build root (`204/204`).
- Working tree note: `palette.dat` remains locally modified (data file only).

### New findings (post-remediation)

1. Path resolution is still relative-path based and can remain launch-directory sensitive.
   - Files:
     - `src/path_resolver.cpp:31-45`
   - Detail:
     - Resolver still tries `"config/...`, `"../config/..."`, and local names.
     - This improves resiliency but does not fully anchor to executable/base path.
   - Impact:
     - Running binaries from unexpected working directories can still resolve unintended files.

2. Source file is included as implementation (`.cpp` include), which is fragile.
   - Files:
     - `src/gamemap.cpp:5` (`#include "tempmap.cpp"`)
   - Impact:
     - Tight coupling of translation units and potential ODR/build hygiene issues.

3. Production warning debt remains in core runtime code.
   - Files:
     - `src/renderer.h:51-59`
     - `src/renderer.cpp:12-21`
     - `src/controller.cpp:19`
     - `tile-maker/tile_sheet.c:188-190`
   - Detail:
     - Constructor member initialization order warnings (`-Wreorder`) in `Renderer`.
     - Unused parameter warnings (`FireProjectile(Player&)`, `double_clicked`).

4. SDL init check style is inconsistent with SDL3 bool semantics in multiple places.
   - Files:
     - `shared/sdl_framework/sdl_context.c:37`
     - `shared/tests/test_main.cpp:17`
   - Detail:
     - Current code uses `SDL_Init(...) < 0`; in SDL3 this is a bool-returning API path in this codebase style.
   - Impact:
     - Generates warnings and obscures intent.

5. Test code has C++ standard compatibility and warning noise issues.
   - Files:
     - `shared/tests/unit/test_sdl_context.cpp:45-50` (designated initializers under C++17)
     - `shared/tests/unit/test_palette_manager.cpp:286-290,341-342`
     - `shared/tests/unit/test_sdl_context.cpp:320`
     - `shared/tests/unit/test_double_click.cpp:39,182,197,210,220`
   - Impact:
     - Avoidable warnings make CI signal noisier and hide meaningful regressions.

### Recommended next focus
- Prioritize deterministic path anchoring first (`SDL_GetBasePath` / executable-relative resolution).
- Then clear warning debt in production targets (`Renderer`, SDL init checks, unused params).
- Finally clean test warning noise and enforce cleaner warning policy in CI.
