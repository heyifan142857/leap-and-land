# Leap And Land

A small 2D jumping game written in C with SDL2.

This repository started as my freshman-year C course final project, inspired by the simple timing-based gameplay of WeChat's "Jump Jump". Two years later, I came back to clean it up, fix memory leaks, and make the project easier to build.

## Overview

Leap And Land is a lightweight platform-jumping game where the player charges a jump, lands on randomly generated blocks, and tries to keep the run going for as long as possible.

Main technologies:

- C11
- SDL2
- SDL2_image
- SDL2_ttf
- SDL2_mixer
- CMake

## Features

- Menu, gameplay, and help scenes
- Random block generation with different block types
- Score and difficulty progression
- Sound effects and background music
- Resource cleanup improvements and memory leak fixes
- Cached image and text rendering for better repeated draw performance

## Controls

### Menu

- `Up / Down / Left / Right` or `W / S`: move selection
- `Space` or `Enter`: confirm
- `Esc`: quit

### In Game

- `Hold Space`: charge jump power
- `Release Space`: jump
- `R`: toggle free jump mode
- `Space` after failing: restart
- `Esc`: quit

### Help Screen

- `Space`: return to the menu
- `Esc`: quit

## Project Structure

```text
.
├── docs/       # Maintenance notes and update logs
├── include/    # Headers
├── src/        # Source files
├── res/        # Images, audio, and fonts
├── LICENSE
└── README.md
```

## Requirements

To build the project, you need:

- A C compiler with C11 support
- CMake 3.16 or newer
- `pkg-config`
- SDL2 development libraries:
  - SDL2
  - SDL2_image
  - SDL2_ttf
  - SDL2_mixer

## Build And Run

### Important Runtime Note

The game loads assets from `./res/...`, so run the executable from the repository root, or make sure the `res/` folder is available relative to your working directory.

### Option 1: CMake

This is the recommended way to build the project.

```bash
cmake -S . -B build
cmake --build build
```

Then run the game from the repository root:

```bash
./build/src/Leap_And_Land
```

On Windows, the executable is typically:

```bash
./build/src/Leap_And_Land.exe
```

### Option 2: Direct Compile

Useful if you want a one-command build with `gcc` or `clang` in a Unix-like shell.

```bash
gcc -std=c11 -Wall -Wextra -pedantic \
  $(pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer) \
  src/main.c src/common.c src/menu.c src/game.c src/help.c src/kun.c \
  src/utils/input.c src/utils/display.c src/utils/audio.c \
  $(pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer) \
  -lm -Iinclude -o leap_and_land
```

Run it from the repository root:

```bash
./leap_and_land
```

## Cross-Platform Setup Notes

### Linux

Install SDL2, SDL2_image, SDL2_ttf, SDL2_mixer, CMake, and `pkg-config` with your distribution's package manager, then use the CMake or direct compile steps above.

### macOS

Install dependencies with Homebrew or your preferred package manager, then build with the same CMake or direct compile commands:

```bash
brew install cmake pkg-config sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

### Windows

The easiest path is MSYS2 + MinGW-w64, because this project uses `pkg-config` in CMake.

Recommended workflow:

1. Install MSYS2.
2. Open the MinGW 64-bit shell.
3. Install a MinGW toolchain, CMake, `pkg-config`, and the SDL2 development packages.
4. Build with:

```bash
cmake -S . -B build
cmake --build build
```

Then run the executable from the repository root so the `res/` directory can be found.

## Development Notes

- Assets are stored under `res/`.
- The project currently uses relative resource paths in code.
- Recent maintenance work also added cached UI rendering and improved resource cleanup.

## Update Log

- [2026-03-26 update log](docs/update-log-2026-03-26.md)

## Acknowledgements

- Thanks to my coursework experience for the original version of this project.
- Thanks to Codex (GPT-5.4) for helping me revisit this project two years later, fix the memory leaks, and clean up parts of the code with some lightweight refactoring.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
