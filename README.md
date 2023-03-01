[![Windows](https://github.com/JacobDomagala/Looper/actions/workflows/windows.yml/badge.svg)](https://github.com/JacobDomagala/Looper/actions/workflows/windows.yml?query=branch%3Amaster)
[![Ubuntu](https://github.com/JacobDomagala/Looper/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/JacobDomagala/Looper/actions/workflows/ubuntu.yml?query=branch%3Amaster)

# Looper
2D Top-down shooter game (and level editor) written in modern C++.

## Build

Looper is CMake based project working both on Linux (Ubuntu) and Windows. To build it, you will need at least C++20 compiler and CMake version 3.22.
While most of the dependencies will be handled by Conan/CMake, it's required that you have Vulkan installed on your machine.</br>

Typical build process would look like this:
```bash
# Create build directory
mkdir build && cd build

# Use newer ABI
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
conan install .. --output-folder=build --build=missing --settings=build_type=Release

# Generate build system for Windows/Linux
cmake -G "Ninja" ..

# Build
cmake --build .
```
