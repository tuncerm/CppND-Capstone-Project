# TODO From Project Analysis

## Status
Reopened after follow-up analysis (`2026-03-08`).

- Baseline remediation remains complete.
- New TODOs below are focused on warning-quality, portability, and path determinism.

## P0 - Path and runtime determinism
- [ ] Replace relative-only path probing in `src/path_resolver.cpp` with executable/base-path anchored resolution.
- [ ] Keep config/map path fallback behavior, but make base directory explicit and deterministic.

## P1 - Build warning quality (production code)
- [ ] Resolve `Renderer` member initialization order warnings by aligning declaration and initializer order.
- [ ] Remove/consume unused parameters in `src/controller.cpp` and `tile-maker/tile_sheet.c`.
- [ ] Update SDL init checks to SDL3-consistent bool style in shared runtime/test code.

## P2 - Test warning cleanup and portability
- [ ] Replace C++20 designated initializers in `shared/tests/unit/test_sdl_context.cpp` with C++17-compatible initialization.
- [ ] Clean warning noise in tests (`-Woverflow`, `-Wmaybe-uninitialized`, `-Wnarrowing`, unused locals).
- [ ] Keep test behavior unchanged while reducing warning volume.

## P3 - Build hygiene
- [ ] Stop including `tempmap.cpp` directly in `src/gamemap.cpp`; move to header/constant declaration model.
- [ ] Ensure data ownership is explicit and translation-unit boundaries remain clean.
