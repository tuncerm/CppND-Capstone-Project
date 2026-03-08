# Active TODO

## Current items
- [ ] Review strict cppcheck pull-request reports over 1-2 cycles and set `CPPCHECK_STRICT_BLOCKING=true` to enforce blocking mode.
- [x] Implement packed subtile format from `docs/map_format.md` (`tile_id:8` + `spec:8` with health/destruction_mode/movement bits).
- [ ] Add damage-type routing in gameplay (`normal/heavy/special`) to use all `destruction_mode` values.
- [ ] Integrate tile-maker pixel data into PlayGame renderer for real tile-asset map rendering (replace color-tinted subtile placeholders).
- [x] Enforce warning-drift guard in CI builds (`ENABLE_WARNINGS_AS_ERRORS`) and fix surfaced warnings.
- [x] Centralize cppcheck profiles/suppressions and upload strict report artifacts in CI.
- [x] Add strict cppcheck report summary generation in CI step summary for faster review.
- [x] Harden file-utils tests with unique temp file/directories to avoid cross-run collisions.
- [x] Add severity-threshold gate (`error=0`, `warning=0`) for strict cppcheck blocking mode.
- [x] Isolate config-manager test file I/O to unique temp directories.

## Notes
- Full analysis context: `docs/project_analysis.md`
