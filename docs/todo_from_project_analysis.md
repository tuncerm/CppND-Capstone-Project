# TODO From Project Analysis

## Status
Closed after implementation pass (`2026-03-08`).

- Path/config resolution is now executable-base anchored.
- Runtime/test warning debt identified in follow-up has been addressed.
- CI is now split by target family with an additional non-blocking strict cppcheck profile for PRs.

## P0 - Path and runtime determinism
- [x] Replace relative-only path probing in `src/path_resolver.cpp` with executable/base-path anchored resolution.
- [x] Keep config/map path fallback behavior, but make base directory explicit and deterministic.

## P1 - Build warning quality (production code)
- [x] Resolve `Renderer` member initialization order warnings by aligning declaration and initializer order.
- [x] Remove/consume unused parameters in `src/controller.cpp` and `tile-maker/tile_sheet.c`.
- [x] Update SDL init checks to SDL3-consistent bool style in shared runtime/test code.

## P2 - Test warning cleanup and portability
- [x] Replace C++20 designated initializers in `shared/tests/unit/test_sdl_context.cpp` with C++17-compatible initialization.
- [x] Clean warning noise in tests (`-Woverflow`, `-Wmaybe-uninitialized`, `-Wnarrowing`, unused locals).
- [x] Keep test behavior unchanged while reducing warning volume.

## P3 - Build hygiene
- [x] Stop including `tempmap.cpp` directly in `src/gamemap.cpp`; move to header/constant declaration model.
- [x] Ensure data ownership is explicit and translation-unit boundaries remain clean.

## P4 - CI follow-up implementation
- [x] Split CI builds by target family (`PlayGame`, `PaletteMaker`, `TileMaker`).
- [x] Isolate shared tests/cppcheck into a dedicated CI job.
- [x] Add stricter non-blocking cppcheck profile for pull requests.
