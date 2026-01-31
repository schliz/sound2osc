# Build Instructions

## Prerequisites
- CMake 3.16+
- C++17 compliant compiler (GCC 9+, Clang 10+, MSVC 2019+)
- Qt 6.2+ (Core, Network, Multimedia, GUI components)
- Ninja (recommended)

## Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `SOUND2OSC_BUILD_GUI` | Build the Qt GUI application | `ON` |
| `SOUND2OSC_BUILD_HEADLESS` | Build the headless CLI application | `OFF` |
| `SOUND2OSC_BUILD_TESTS` | Build unit tests | `OFF` |
| `SOUND2OSC_ENABLE_COVERAGE` | Enable code coverage generation | `OFF` |
| `SOUND2OSC_AUDIO_BACKEND` | Audio backend to use (`Qt`, `Miniaudio`) | `Qt` |

## Audio Backends

### Qt Multimedia (Default)
Uses `Qt6::Multimedia` module.
- Standard backend for desktop use with full device enumeration and format support provided by Qt.
- Best for the GUI application.

### Miniaudio
Uses `miniaudio` (single-header library).
- **Optimized for low latency**: Uses a direct event-driven loop triggered by audio callbacks.
- **Minimal Dependencies**: Does not require `Qt6::Multimedia`. Useful for headless environments.
- Recommended for Raspberry Pi / Embedded devices running the headless daemon.

## Build Examples

### 1. Standard Desktop Build (GUI + Qt Audio)
```bash
cmake -B build -G Ninja
cmake --build build
```

### 2. Headless Build (CLI + Miniaudio)
This configuration removes GUI dependencies and uses the lightweight audio backend.

```bash
cmake -B build-headless -G Ninja \
    -DSOUND2OSC_BUILD_GUI=OFF \
    -DSOUND2OSC_BUILD_HEADLESS=ON \
    -DSOUND2OSC_AUDIO_BACKEND=Miniaudio
cmake --build build-headless
```

### 3. Debug Build
```bash
cmake -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```
