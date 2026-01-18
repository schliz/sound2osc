# Sound2OSC Modernization Plan

**Status:** Phase 1 - Complete, Phase 2 - Ready  
**Last Updated:** 2026-01-18  
**Target:** Modern cross-platform build with Qt6, C++17, CMake

---

## Overview

This document outlines the comprehensive plan to modernize the sound2osc project from its 2019 Qt5/qmake codebase to a modern, maintainable, cross-platform application ready for future enhancements including headless operation and web-based UI.

### Goals

1. **Modern Build System** - CMake for cross-platform support (Linux/Windows/macOS)
2. **Qt6 Migration** - Update to Qt 6.x with C++17
3. **Clean Architecture** - Separate core library from UI frontends
4. **Future-Ready** - Enable headless and web-ui options
5. **Developer Experience** - Clear structure, easy onboarding, CI/CD

---

## Current State Analysis

### Repository Issues
- All source code in single `src/` directory (92 files mixed together)
- 87MB Windows installer zip checked into repository
- No separation between UI and business logic
- FFT library embedded in source tree without clear attribution
- QML files mixed with C++ code
- Build artifacts can pollute source directory

### Code Architecture
- Qt5/C++11 codebase (last updated 2019)
- Uses deprecated `QLinkedList`
- Monolithic GUI application (no headless support)
- qmake build system (outdated)
- Hard-coded organization name "ETC" in source

### Logical Components Identified

| Component | Files | Description |
|-----------|-------|-------------|
| **Audio Input** | `AudioInputInterface.h`, `QAudioInputWrapper.*`, `MonoAudioBuffer.*` | Audio capture abstraction |
| **FFT/DSP** | `FFTAnalyzer.*`, `FFTRealWrapper.h`, `ScaledSpectrum.*`, `ffft/` | Signal processing |
| **Trigger System** | `TriggerFilter.*`, `TriggerGenerator.*`, `TriggerOscParameters.*` | Event generation |
| **BPM Detection** | `BPMDetector.*`, `BPMTapDetector.*`, `BPMOscControler.*` | Beat detection |
| **OSC Protocol** | `OSCParser.*`, `OSCMessage.*`, `OSCMapping.*`, `OSCNetworkManager.*` | Network messaging |
| **GUI Controllers** | `MainController.*`, `TriggerGuiController.*` | Application logic |
| **GUI/QML** | `main.cpp`, `qml/` directory | User interface |
| **Utilities** | `QCircularBuffer.h`, `utils.h`, `versionInfo.h` | Shared helpers |

---

## Target Directory Structure

```
sound2osc/
├── CMakeLists.txt                 # Root CMake configuration
├── README.md
├── LICENSE
├── MODERNIZATION_PLAN.md         # This file
├── AGENTS.md                     # AI assistant guidance
│
├── cmake/                         # CMake modules and scripts
│   ├── Qt6Config.cmake
│   ├── Platform.cmake            # Platform-specific settings
│   └── CompilerWarnings.cmake    # Modern warning flags
│
├── docs/                          # Documentation
│   ├── building/                 # Build instructions per platform
│   ├── manuals/                  # User manuals (PDFs)
│   ├── changelog.md
│   ├── screenshots/
│   └── examples/                 # Example configs, magic sheets
│
├── third_party/                  # External dependencies
│   └── ffft/                     # FFT library with attribution
│       ├── README.md
│       └── include/ffft/
│
├── libs/                         # Core libraries (UI-independent)
│   ├── CMakeLists.txt
│   └── sound2osc-core/           # Main core library
│       ├── CMakeLists.txt
│       ├── include/sound2osc/
│       │   ├── audio/            # Audio input abstraction
│       │   ├── dsp/              # FFT and spectrum analysis
│       │   ├── trigger/          # Trigger generation
│       │   ├── bpm/              # BPM detection
│       │   ├── osc/              # OSC protocol handling
│       │   └── core/             # Utilities, buffers, version
│       └── src/
│           ├── audio/
│           ├── dsp/
│           ├── trigger/
│           ├── bpm/
│           ├── osc/
│           └── core/
│
├── apps/                          # Applications/Frontends
│   ├── CMakeLists.txt
│   ├── gui/                      # Qt6 GUI application
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp
│   │   ├── src/
│   │   │   └── controllers/      # GUI-specific controllers
│   │   ├── qml/
│   │   │   ├── main.qml
│   │   │   ├── components/       # Reusable QML components
│   │   │   ├── dialogs/
│   │   │   ├── views/
│   │   │   └── style/            # Dark theme components
│   │   └── resources/
│   │       ├── images/
│   │       ├── icons/
│   │       ├── qml.qrc
│   │       └── images.qrc
│   │
│   ├── headless/                 # CLI/daemon (future)
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   │
│   └── web-ui/                   # Web interface (future)
│       └── README.md
│
├── tests/                         # Testing
│   ├── CMakeLists.txt
│   ├── unit/
│   └── integration/
│
├── scripts/                       # Build and utility scripts
│   ├── build.sh                  # Cross-platform build wrapper
│   ├── install-deps-debian.sh
│   ├── install-deps-macos.sh
│   ├── install-deps-windows.ps1
│   └── package/                  # Packaging scripts
│
└── .github/                       # CI/CD
    └── workflows/
        └── build.yml             # GitHub Actions multi-platform
```

---

## Implementation Phases

### Phase 1: Repository Cleanup & Build System (Current)

**Status:** Complete

- [x] Create modernization plan document
- [x] Create AGENTS.md
- [x] Remove `src/sound2light_installer_data_windows.zip` from git history
- [x] Create new directory structure
- [x] Set up root CMakeLists.txt with Qt6 support
- [x] Create cmake/ helper modules
- [x] Move and reorganize source files to new structure
- [x] Create libs/sound2osc-core CMake configuration
- [x] Create apps/gui CMake configuration
- [x] Update .gitignore for CMake out-of-source builds
- [x] Update build documentation for CMake
- [x] Verify build works on Debian 13/14
- [x] Remove legacy src/ and doc/ directories

**Deliverables:**
- Working CMake build on Linux
- Clean repository structure
- Updated documentation

### Phase 2: Qt6 & C++17 Migration

**Status:** Not Started

- [ ] Update Qt5 APIs to Qt6 equivalents
- [ ] Replace deprecated `QLinkedList` with `QList` or `std::list`
- [ ] Update QML imports to Qt6 syntax
- [ ] Modernize signal/slot connections (use function pointers)
- [ ] Apply C++17 features:
  - `std::optional` for optional values
  - `if constexpr` for compile-time branching
  - Structured bindings
  - `std::filesystem` for path handling
- [ ] Add `override` keyword consistently
- [ ] Improve `const` correctness
- [ ] Replace raw pointers with smart pointers where appropriate

**Deliverables:**
- Qt6-native codebase
- Modern C++17 code
- No deprecation warnings

### Phase 3: Architecture Refactoring

**Status:** Not Started

- [ ] Extract business logic from `MainController` to core library
- [ ] Define clean interfaces between core and GUI
- [ ] Make audio backend abstraction complete (for future backends)
- [ ] Create headless application skeleton
- [ ] Implement configuration system (JSON/TOML)
- [ ] Make organization/app name configurable
- [ ] Add proper logging system

**Deliverables:**
- `libsound2osc-core` usable independently
- Working headless prototype
- Configurable application identity

### Phase 4: Cross-Platform & CI/CD

**Status:** Not Started

- [ ] Set up GitHub Actions workflow
- [ ] Test and fix Windows build
- [ ] Test and fix macOS build
- [ ] Create AppImage for Linux
- [ ] Create DMG for macOS
- [ ] Create Windows installer (NSIS or WiX)
- [ ] Automated release pipeline
- [ ] Add unit tests with CTest

**Deliverables:**
- Multi-platform CI/CD
- Automated release artifacts
- Test coverage

---

## Technical Decisions

### Build System: CMake

**Rationale:**
- Industry standard for C++ projects
- Excellent Qt6 integration via `find_package(Qt6)`
- Supports all major IDEs (CLion, VS Code, Qt Creator, Visual Studio)
- Native support for out-of-source builds
- Built-in testing support (CTest)

### Qt Version: Qt 6.x

**Rationale:**
- Active development and LTS support
- Better performance (especially QML)
- Required for future Qt features
- Modern C++ integration
- Qt5 is in maintenance mode

### C++ Standard: C++17

**Rationale:**
- Good balance of features and compiler support
- Required for Qt6
- `std::optional`, `std::filesystem`, structured bindings
- Broad compiler support (GCC 7+, Clang 5+, MSVC 2017+)

### Library Architecture: Separate Core

**Rationale:**
- Enables headless operation without pulling in Qt GUI
- Clean separation of concerns
- Core can be tested independently
- Future web-ui can link to same core
- Other applications can use the library

---

## Migration Notes

### Files to Remove from Git

```
src/sound2light_installer_data_windows.zip  # 87MB, should be in releases
```

### Files to Relocate

| Original | New Location |
|----------|--------------|
| `src/ffft/` | `third_party/ffft/include/ffft/` |
| `src/main.cpp` | `apps/gui/main.cpp` |
| `src/MainController.*` | `apps/gui/src/controllers/` |
| `src/TriggerGuiController.*` | `apps/gui/src/controllers/` |
| `src/qml/` | `apps/gui/qml/` |
| `src/images/` | `apps/gui/resources/images/` |
| `src/*.qrc` | `apps/gui/resources/` |
| `src/OSC*.*` | `libs/sound2osc-core/src/osc/` |
| `src/BPM*.*` | `libs/sound2osc-core/src/bpm/` |
| `src/Trigger*.*` (non-GUI) | `libs/sound2osc-core/src/trigger/` |
| `src/FFT*.*` | `libs/sound2osc-core/src/dsp/` |
| `src/ScaledSpectrum.*` | `libs/sound2osc-core/src/dsp/` |
| `src/*Audio*.*` | `libs/sound2osc-core/src/audio/` |
| `src/MonoAudioBuffer.*` | `libs/sound2osc-core/src/audio/` |
| `src/QCircularBuffer.h` | `libs/sound2osc-core/include/sound2osc/core/` |
| `src/utils.h` | `libs/sound2osc-core/include/sound2osc/core/` |
| `src/versionInfo.h` | `libs/sound2osc-core/include/sound2osc/core/` |
| `doc/` | `docs/` |

### Qt5 to Qt6 API Changes

| Qt5 | Qt6 |
|-----|-----|
| `QLinkedList` | `QList` or `std::list` |
| `QRegExp` | `QRegularExpression` |
| `QTextCodec` | `QStringConverter` |
| `qApp->desktop()` | `QGuiApplication::primaryScreen()` |
| `QMouseEvent::pos()` | `QMouseEvent::position().toPoint()` |
| `qt5_add_resources()` | `qt_add_resources()` |

---

## Dependencies

### Debian 13/14

```bash
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-declarative-dev \
    qt6-quickcontrols2-dev \
    libgl1-mesa-dev \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-dialogs \
    qml6-module-qtquick-window \
    qml6-module-qtmultimedia
```

### macOS (Homebrew)

```bash
brew install cmake ninja qt@6
```

### Windows (vcpkg or Qt Installer)

```powershell
# Using Qt Online Installer recommended
# Or via vcpkg:
vcpkg install qt6-base qt6-multimedia qt6-declarative qt6-quickcontrols2
```

---

## References

- [Qt 5 to Qt 6 Migration Guide](https://doc.qt.io/qt-6/portingguide.html)
- [CMake Qt6 Documentation](https://doc.qt.io/qt-6/cmake-manual.html)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)
- [C++17 Features](https://en.cppreference.com/w/cpp/17)
