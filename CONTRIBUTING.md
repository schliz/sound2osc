# Contributing to sound2osc

## Project Structure

- `apps/`: Application entry points (GUI and Headless).
- `libs/sound2osc-core/`: Core logic (Audio, DSP, OSC, Config). Independent of GUI.
- `tests/`: Unit and integration tests.

## Build Instructions

### Prerequisites
- Qt 6.2+ (Modules: Core, Network, Multimedia)
- CMake 3.23+
- FFTW3 (`libfftw3-dev`)
- C++17 Compiler

### Build Commands

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build
```

## Development Guidelines

- **Code Style**: Follow the existing C++17 / Qt style.
- **Architecture**: The core library (`libs/sound2osc-core`) must remain UI-agnostic. Use Qt Signals/Slots for communication between the core and the application layer.
- **Real-time Safety**: Audio processing occurs on a high-priority thread. Avoid memory allocations and blocking I/O in `process()` or `analyze()` methods.
- **Testing**: Run tests before submitting PRs:
  ```bash
  cd build
  ctest
  ```

## Pull Requests

Please ensure your changes compile and pass tests. Follow the [Conventional Commits](https://www.conventionalcommits.org/) specification for commit messages.
