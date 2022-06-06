
![Windows](https://github.com/JacobDomagala/DGame/workflows/Windows%20Full%20Build/badge.svg)
![Ubuntu](https://github.com/JacobDomagala/DGame/workflows/Ubuntu%20Full%20Build/badge.svg)

# Looper
2D Top-down shooter game (and level editor) written in modern C++.

## Build

Looper is CMake based project working both on Linux (Ubuntu) and Windows. To build it, you will need at least C++17 compiler and CMake version 3.18. </br>

Typical build process would look like this:
```bash
# Create build directory
mkdir build && cd build

# Generate build system for Windows/Linux
cmake -G "Visual Studio 16 2019" ..
cmake -G "Ninja" ..

# Build
cmake --build .
```