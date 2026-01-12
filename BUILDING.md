# GoldPedal Build Instructions

## Prerequisites
- CMake 3.22+
- Visual Studio 2022 (Windows) or Xcode (macOS)
- JUCE submodule initialized: `git submodule update --init --recursive`

## Windows (Visual Studio)
1. Open a Developer PowerShell in the project root.
2. Configure the project:
   `cmake -B build -S . -G "Visual Studio 17 2022" -A x64`
3. Build:
   `cmake --build build --config Release`
4. The Standalone app and VST3 will be in `build` under the JUCE build outputs.

## macOS (Xcode)
1. Open Terminal in the project root.
2. Configure the project:
   `cmake -B build -S . -G Xcode`
3. Build:
   `cmake --build build --config Release`
4. The Standalone app and VST3 will be in `build` under the JUCE build outputs.
