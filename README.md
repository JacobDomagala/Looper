[![Windows](https://github.com/JacobDomagala/Looper/actions/workflows/windows.yml/badge.svg)](https://github.com/JacobDomagala/Looper/actions/workflows/windows.yml?query=branch%3Amaster)
[![Ubuntu](https://github.com/JacobDomagala/Looper/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/JacobDomagala/Looper/actions/workflows/ubuntu.yml?query=branch%3Amaster)

# Looper
Looper is a game engine with an integrated editor and a 2D type game with a level editor, all written in modern C++20. It uses Vulkan for rendering and ImGui/glfw3 for UI and window/input handling. The project is compatible with Ubuntu and Windows.


[![Watch the video](https://raw.githubusercontent.com/wiki/JacobDomagala/Looper/Looper_github.gif)](https://www.youtube.com/watch?v=Qh-vOKMPQGQ)

## Requirements
- C++20 compatible compiler (e.g. GCC, Clang, MSVC)
- CMake version 3.22 or higher
- Vulkan SDK
- Conan package manager

## Build
Looper is a CMake-based project working on both Linux (Ubuntu) and Windows. To build it, you will need at least a C++20 compiler and CMake version 3.22. Most of the dependencies will be handled by Conan/CMake, but it's required that you have Vulkan installed on your machine.

The typical build process would look like this:

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

## Usage
1. Run the compiled binary to launch the Looper game engine and editor.
2. Create custom levels in the level editor.
3. Save and load levels.
4. Switch to "game-mode" to play your custom levels in the 2D type game.

## Contributing
If you would like to contribute to the project, please fork the repository and submit a pull request with your proposed changes. We welcome any improvements or new features that enhance the functionality and user experience of Looper.

## License
Please refer to the LICENSE file in the repository for details on the licensing of this project.
