# Update Log - 2026-03-26

## Summary

This update focused on stability, cleanup, performance improvements in repeated UI rendering, and repository presentation for GitHub.

## Changes

- Fixed multiple memory leaks related to:
  - text textures
  - help screen textures
  - replaced block textures
  - keyboard state allocation
  - music and sound resource cleanup
- Refactored repeated block creation and linked-list update logic in the gameplay code
- Added safer SDL, SDL_image, SDL_ttf, and SDL_mixer resource handling
- Added image and text caching for repeated `display_image()` and `display_font()` calls
- Added a cache cleanup step before application shutdown
- Fixed repeated `YOU LOSE` console spam so failure is logged only once per run
- Replaced the temporary top-10 leaderboard with a single local best-score record to avoid UI overlap
- Added fail-screen feedback for either a new best score or the current stored best score
- Marked assisted-mode runs as ineligible for best-score submission and kept the English fail-screen notice
- Updated `CMakeLists.txt` to use `pkg-config`-based SDL dependency discovery
- Rewrote `README.md` in English for a cleaner GitHub-ready presentation

## Notes

- The project still uses relative paths such as `./res/...`, so the executable should be launched from the repository root unless the resource directory layout is preserved.
- The new display cache uses bounded in-memory caches and evicts older entries when limits are reached.
- Local score data is stored in `./best_score.txt`, while legacy `leaderboard.txt` is also ignored by Git if it exists locally.

## Verification

- Verified with local `gcc` compilation using SDL2, SDL2_image, SDL2_ttf, and SDL2_mixer
- Verified that the game starts in a dummy SDL environment without immediate startup failures
