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
5. CI workflow improvements
   - Split CI builds by target family (`PlayGame`, `PaletteMaker`, `TileMaker`).
   - Isolated shared tests + analysis job.
   - Added non-blocking strict cppcheck profile for pull requests.
   - Added shared test labels and CI label filtering (`ctest -L shared`).
   - Added warnings-as-errors enforcement option (`ENABLE_WARNINGS_AS_ERRORS`) and enabled it in CI.
   - Centralized cppcheck invocation via `tools/run_cppcheck.sh` and upload strict cppcheck report artifacts in PR CI.
   - Added strict cppcheck XML summary generation for CI step summaries (`tools/summarize_cppcheck_xml.py`).

## Current verification snapshot
- Build: `cmake --build build --clean-first` passes.
- Tests: `ctest --output-on-failure` and `ctest -L shared --output-on-failure` pass (`204/204`).

## Remaining recommendations (non-blocking)
- Monitor strict cppcheck pull-request output for 1-2 cycles, then promote stable checks from non-blocking to blocking.

Tracked as active items in `docs/todo.md`.
