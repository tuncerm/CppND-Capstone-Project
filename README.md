# CPPND: Capstone Project

This is my Capstone Project for the [Udacity C++ Nanodegree Program](https://www.udacity.com/course/c-plus-plus-nanodegree--nd213). The starter code was provided by Udacity.  
<img src="playgame.gif"/>

This project integrates many of the C++ concepts taught throughout the program. It's a showcase of my ability to independently build applications using modern C++.

I extended the [Snake Game](https://github.com/udacity/CppND-Capstone-Snake-Game), applying good software design and engineering principles.

## Dependencies for Running Locally

- CMake ≥ 3.21  
  Install: https://cmake.org/install/  
  macOS: `brew install cmake`

- Make ≥ 4.1 (Linux/macOS), 3.81 (Windows)  
  Windows: install via MSYS2 or http://gnuwin32.sourceforge.net/packages/make.htm  
  macOS: `xcode-select --install` or `brew install make`

- SDL3  
  Install: https://github.com/libsdl-org/SDL  
  macOS: `brew install sdl3`  
  Linux: `apt install libsdl3-dev` (or equivalent)

- GCC/G++ ≥ 7  
  macOS: via `xcode-select --install`  
  Windows: recommended via MSYS2 - https://www.msys2.org/

## Build Instructions

1. Clone this repo.
2. Create and enter a build directory:
   mkdir build && cd build
3. Build:
   cmake .. && make
4. Run:
   ./PlayGame
