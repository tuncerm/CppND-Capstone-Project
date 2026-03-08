# TODO From Project Analysis (2026-03-07)

## P0 - Correctness and crash prevention
- [ ] Fix `Character::IsAlive(bool)` no-op setter in `src/character.h`.
- [ ] Add bounds-safe access for `GameMap::GetElement` and guard empty-map access in `AreaIsAvailable`.
- [ ] Validate loaded map dimensions against configured grid dimensions at startup.
- [ ] Remove hardcoded AI map size (`20x32`) and add bounds checks in `AICentral`.
- [ ] Standardize runtime asset/config path resolution (avoid working-directory-dependent relative paths).

## P1 - Shared library contract and test stability
- [ ] Refactor `shared/sdl_framework/sdl_context.c` so `sdl_cleanup_context` does not call global `SDL_Quit`.
- [ ] Define and enforce SDL lifecycle ownership (app-level/global refcount policy).
- [ ] Fix `file_get_filename` path-separator handling (`/` and `\`) in `shared/utilities/file_utils.c`.
- [ ] Align `file_create_backup` behavior with expected contract (`append .bak` vs replace extension).
- [ ] Fix `file_sanitize_filename` to preserve null terminator.
- [ ] Align `double_click` state transitions with expected semantics (reset after recognized double-click/timeouts).
- [ ] Fix `UI_BUTTON_NORMAL` state semantics (`0x00` flag check issue).
- [ ] Resolve `palette_manager_save(..., nullptr)` contract mismatch between implementation and tests.

## P2 - Performance and architecture cleanup
- [ ] Remove per-frame `std::async` spawn + immediate wait in `src/game.cpp`.
- [ ] Change `Renderer::Render` enemy parameter to const reference (avoid copy each frame).
- [ ] Cache config-derived render constants/colors instead of repeated per-frame lookups.
- [ ] Add validation and fallback strategy for malformed config values.

## P3 - Config-driven behavior consistency
- [ ] Replace hardcoded tool filenames (`tiles.dat`, `palette.dat`) with config values everywhere.
- [ ] Ensure `palette-maker` and `tile-maker` use consistent config path strategy.
- [ ] Add startup summary logging for effective config values and resolved file paths.

## P4 - Text/UI correctness and cleanup
- [ ] Fix glyph lookup bounds in `palette-maker/ui.c` and `tile-maker/ui.c` (`GLYPH_COUNT`).
- [ ] Deduplicate duplicated bitmap-font code between `palette-maker` and `tile-maker`.

## P5 - Build/test workflow
- [ ] Enable CTest at top-level CMake so root `ctest` discovers tests.
- [ ] Add CI pipeline for build + tests on main targets.
- [ ] Add static analysis step (`clang-tidy`/`cppcheck`) and sanitizer debug configuration.

## Suggested execution order
1. P0
2. P1
3. P2
4. P3
5. P4
6. P5
