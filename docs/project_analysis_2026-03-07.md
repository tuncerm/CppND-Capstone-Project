# Project Analysis Report (2026-03-07)

## Scope reviewed
- `.rooroo/tasks` context from previous work:
  - `ROO#TASK_20250626184833_4F1A8C` (initial architecture analysis)
  - `ROO#TASK_20250626185944_E4D3F2` (palette-maker build issues)
  - `ROO#TASK_20250626193815_D3E4F1` (JSON config integration request)
  - `ROO#TASK_20250626195551_A1B2C3` (palette-maker build failure follow-up)
- Existing docs in `docs/`:
  - `docs/constants_guide.md`
- Core code areas:
  - `src/`, `shared/`, `palette-maker/`, `tile-maker/`

## Build and test status
- Build command: `cmake --build build --parallel 4`
  - Result: success for `PlayGame`, `PaletteMaker`, `TileMaker`, and `shared_components_tests`.
- Test command at repo build root: `ctest --output-on-failure`
  - Result: `No tests were found!!!`
  - Root cause: top-level CMake does not call `enable_testing()`/`include(CTest)` (`CMakeLists.txt:1-80`), while tests are enabled in subdir only (`shared/CMakeLists.txt:69-73`).
- Test command in shared tests dir: `build/shared/tests/ctest --output-on-failure`
  - Result: 20 failed tests out of 204.
  - Reproducible failure groups: `DoubleClickTest`, `FileUtilsTest`, `SDLContextTest`, `PaletteManagerTest`, `UIButtonTest`, `TextRendererTest`.

## Confirmed issues

### 1) `Character::IsAlive(bool)` setter is a no-op
- File: `src/character.h:28`
- Problem: `void IsAlive(bool alive) { alive = alive; }` assigns parameter to itself.
- Impact: Callers cannot change `_alive`; any future death/life logic will silently fail.
- Recommendation: assign to member (`_alive = alive;`) and consider renaming to `SetAlive`.

### 2) Map access can go out-of-bounds
- Files:
  - `src/gamemap.h:16` (`GetElement` has no bounds checks)
  - `src/gamemap.cpp:7-13` (`AreaIsAvailable` reads `_map[0].size()` without guarding empty map)
  - `src/renderer.cpp:71-80` (loops using configured width/height, then calls `GetElement`)
- Problem: configured grid size and loaded map dimensions are not validated/aligned.
- Impact: malformed/empty map files or config mismatch can crash.
- Recommendation: validate map dimensions at load time; add safe accessors; fail fast on invalid map.

### 3) Runtime map/config file path fragility
- Files:
  - `src/gamemap.cpp:18` (`"game.map"`)
  - `src/main.cpp:34`, `src/renderer.cpp:59` (`"config/game_config.json"`)
  - `palette-maker/main.c:17` (`"../config/palette_maker_config.json"`)
  - `tile-maker/main.c:71` (`"config/tile_maker_config.json"`)
- Problem: relative paths depend on working directory.
- Impact: binaries launched from different directories may ignore intended assets/config and fall back silently.
- Recommendation: resolve paths from executable directory or a known project root; surface explicit error when files are missing.

### 4) AI map is fixed-size and unchecked
- Files:
  - `src/AICentral.h:30` (`MapObject _map[20][32];`)
  - `src/AICentral.cpp:14-21` (no bounds checks in `AddToMap`/`ReadFromMap`)
- Problem: AI map dimensions are hardcoded and independent of runtime config.
- Impact: if map dimensions change, writes/reads can go out-of-bounds.
- Recommendation: size AI map dynamically from `GameMap`, or validate dimensions before every access.

### 5) Shared SDL cleanup shuts down global SDL unexpectedly
- Files:
  - `shared/sdl_framework/sdl_context.c:98-116` (`sdl_cleanup_context` calls `SDL_Quit`)
  - `shared/tests/utils/test_helpers.cpp:12-28` (expects global SDL to remain initialized)
  - `shared/tests/test_main.cpp:16-19, 34-39` (global SDL init/quit)
- Problem: each context cleanup can kill global SDL state.
- Impact: cross-test contamination and unpredictable multi-context behavior.
- Recommendation: do not call `SDL_Quit` in per-context cleanup; manage SDL lifecycle once (app/global refcount).

### 6) `file_utils` has contract/behavior defects
- Files:
  - `shared/utilities/file_utils.c:71-85` (`file_get_filename` does not handle `/` on Windows paths and returns full path)
  - `shared/utilities/file_utils.c:210-220` (`file_create_backup` replaces extension instead of appending `.bak`)
  - `shared/utilities/file_utils.c:388-395` (`file_sanitize_filename` overwrites string terminator with `_`)
- Related failing tests:
  - `shared/tests/unit/test_file_utils.cpp:67-77`, `149-157`
- Impact: incorrect filenames, missing backups, malformed sanitized names.
- Recommendation: normalize path separator handling, define backup naming contract (`append` vs `replace`), and preserve `'\0'`.

### 7) `double_click` behavior does not match expected state model
- Files:
  - `shared/utilities/double_click.c:25-43`
  - Tests expecting reset semantics: `shared/tests/unit/test_double_click.cpp:59, 75, 120, 142, 162, 269`
- Problem: detector keeps previous-click state after double-click/timeouts.
- Impact: repeated false positives/incorrect click sequence handling.
- Recommendation: clear previous state after successful double-click and define timeout expiration behavior clearly.

### 8) `UI_BUTTON_NORMAL` state semantics are inconsistent
- Files:
  - `shared/ui_framework/ui_button.h:27` (`UI_BUTTON_NORMAL = 0x00`)
  - `shared/ui_framework/ui_button.c:186-192` (`ui_button_has_state` uses bitwise check)
  - Test: `shared/tests/unit/test_ui_button.cpp:66`
- Problem: checking "normal" via bitmask cannot be true when value is zero.
- Impact: state checks/reporting are wrong.
- Recommendation: special-case normal state in `ui_button_has_state` or redefine normal as explicit flag.

### 9) API contract mismatch in palette save behavior
- Files:
  - `shared/palette_manager/palette_manager.c:153-160`
  - Tests: `shared/tests/unit/test_palette_manager.cpp:211, 216`
- Problem: implementation saves to default `"palette.dat"` when filepath is null and no current file; tests expect failure in that case.
- Impact: ambiguous behavior and failing tests.
- Recommendation: choose one contract and align implementation/tests/docs.

### 10) Text glyph lookup drops special characters
- Files:
  - `palette-maker/ui.c:629`
  - `tile-maker/ui.c:530`
- Problem: `font_patterns[char_index < 52 ? char_index : 0]` ignores glyph indices 52-59 even though table has 60 entries.
- Impact: many supported symbols render as space.
- Recommendation: clamp against `GLYPH_COUNT`, not `52`.

### 11) Hot-path performance issues in main game loop/rendering
- Files:
  - `src/game.cpp:57-58` (`std::async` + immediate `get` every frame)
  - `src/renderer.h:41`, `src/renderer.cpp:67` (enemy passed by value)
  - `src/renderer.cpp:71-87, 145-158` (repeated config lookups in per-frame loops)
- Problem: unnecessary allocations/copies/lookups in frame-critical path.
- Impact: avoidable frame-time jitter and lower headroom.
- Recommendation: avoid per-frame `async` spawn, pass enemy by const reference, cache config-derived render values.

### 12) Config values are registered but bypassed in tool workflows
- Files:
  - `tile-maker/main.c:66, 68` (register default filenames)
  - `tile-maker/main.c:244, 255, 337, 344, 446, 451` (hardcoded `"tiles.dat"`/`"palette.dat"`)
  - `palette-maker/ui.c:101-209` (hardcoded `"palette.dat"`)
- Problem: config-driven defaults are not used consistently.
- Impact: configuration file cannot fully control runtime behavior.
- Recommendation: read file paths once from config and route all save/load operations through them.

## Improvement opportunities
- Add a CI matrix that runs:
  - `cmake --build`
  - `ctest` at correct test dir
  - static analysis (`clang-tidy`/`cppcheck`)
  - sanitizer build (`ASan`/`UBSan`) for debug profile.
- Consolidate CMake patterns across app/tool subprojects to reduce duplicated platform-linking logic.
- Strengthen data validation for external assets:
  - map schema (rows/cols/value range)
  - config schema/type checks
  - explicit startup diagnostics with actionable messages.
- Separate SDL lifecycle ownership from feature modules (context vs process).

## Possible next features/expansions
- Dynamic map loading with strict schema + editor integration from `tile-maker`.
- Deterministic replay/seed mode for debugging gameplay behavior.
- Unified asset path manager for game/tool executables.
- Runtime config hot-reload for UI/theme/performance tuning.

## Suggested execution order
1. Fix correctness bugs with highest blast radius:
   - `Character::IsAlive`, `GameMap` bounds/dimension validation, `AICentral` bounds handling.
2. Stabilize shared library contracts:
   - `sdl_context` lifecycle policy, `file_utils` path/sanitize logic, `double_click` semantics.
3. Align tests and contracts:
   - `palette_manager_save` null-path behavior, `UI_BUTTON_NORMAL` checks.
4. Address performance and configuration consistency:
   - renderer/game loop hot-path cleanup, remove hardcoded tool filenames/paths.
