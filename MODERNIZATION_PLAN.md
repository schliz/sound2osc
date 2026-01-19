# Sound2OSC Modernization Plan

**Status:** Phase 1 - Complete, Phase 2 - Complete, Phase 3 - Complete, Phase 4 - In Progress  
**Last Updated:** 2026-01-19  
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

## Current Implementation Summary (as of 2026-01-18)

### Code Statistics
- **Core Library:** 33 files (headers + sources) in `libs/sound2osc-core/`
- **GUI Application:** Located in `apps/gui/`
- **Built Binary:** 787KB executable at `build/bin/sound2osc`
- **Build System:** CMake with Ninja (out-of-source builds)
- **Target:** Qt6 and C++17

### Code Quality Improvements Completed
| Category | Status | Details |
|----------|--------|---------|
| Qt6 Migration | ✓ Complete | All APIs updated, no deprecation warnings |
| C++17 Features | ✓ Partial | constexpr, override, nullptr in use |
| Signal/Slots | ✓ Complete | All converted to function pointer syntax |
| Virtual Destructors | ✓ Complete | Added to all interface classes |
| Type Safety | ✓ 95% | Some minor conversion warnings remain |

### Known Warnings (Non-Critical)
- None. (All warnings have been resolved or suppressed where appropriate)

### Application Functionality
- Audio input capture via Qt Multimedia
- FFT analysis with 4096-sample windows
- Real-time frequency-based trigger generation
- OSC protocol for network control
- BPM tap detection and analysis
- GUI with QML/Qt Quick controls

### Build & Deployment
- **Platform:** Linux (tested), Windows/macOS (CMake files ready)
- **Dependencies:** Qt6 base, Qt6 multimedia, Qt6 declarative
- **CI/CD:** GitHub Actions workflow configured (semantic-release)

---

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

**Status:** Complete (2026-01-18)

- [x] Update Qt5 APIs to Qt6 equivalents
  - Replaced `QVariant::type()` with `typeId()` and `QMetaType` enums in `OSCMessage.cpp`
  - Updated QML `MessageDialog` API (buttons, onButtonClicked) in dialog components
  - Removed deprecated `Qt::AA_EnableHighDpiScaling` (enabled by default in Qt6)
- [x] Replace deprecated `QLinkedList` with `QList` or `std::list`
  - Verified: No QLinkedList usage found in codebase
- [x] Update QML imports to Qt6 syntax
  - Updated `Qt.labs.folderlistmodel` (version removed in Qt6)
- [x] Modernize signal/slot connections (use function pointers)
  - Converted all remaining SIGNAL/SLOT macros to function pointer syntax
  - Updated in: `main.cpp`, `OSCNetworkManager.cpp`, `TriggerFilter.cpp`, controllers
- [x] Apply C++17 features:
  - Used `constexpr` for compile-time constants in `FFTAnalyzer.h`, `ScaledSpectrum.h`
  - Bit-shift operators (`1 << NUM_SAMPLES_EXPONENT`) for compile-time computations
  - ✓ Partial: `std::optional` (future use), `if constexpr` (not needed yet)
  - TODO: `std::filesystem` for path handling
- [x] Add `override` keyword consistently
  - Added to derived classes: `OSCPacketWriter`, `OSCBundleWriter`, `TriggerGenerator`
  - 18 override declarations added across core library
- [x] Improve `const` correctness
  - Updated method signatures and variable declarations
- [ ] Replace raw pointers with smart pointers where appropriate
  - Low priority: Mainly internal pointers with clear ownership
  - `FFTAnalyzer.m_fft` could use `unique_ptr` (future optimization)

**Deliverables:**
- ✓ Qt6-native codebase (fully modernized)
- ✓ Modern C++17 code (constexpr, override, nullptr)
- ✓ No deprecation warnings (only minor conversion warnings in non-critical code)
- ✓ Binary builds and runs successfully (787KB executable)

### Phase 3: Architecture Refactoring

**Status:** Complete (2026-01-18)

#### Task 1: Cross-Platform Logging System
- [x] Create `Logger.h` with cross-platform support
- [x] Implement `Logger.cpp` with:
  - Log levels (Debug, Info, Warning, Error, Critical)
  - Console, file, and syslog/Event Log/unified logging output
  - Thread-safe logging
  - Platform-specific output (Linux syslog, Windows Event Log, macOS unified logging)
- [x] Integrate into core library CMakeLists.txt

**Files Created:**
- `libs/sound2osc-core/include/sound2osc/logging/Logger.h`
- `libs/sound2osc-core/src/logging/Logger.cpp`

#### Task 2: Configuration Abstraction Layer
- [x] Create `ConfigStore.h` - abstract interface for configuration storage backends
- [x] Implement `JsonConfigStore.h/cpp` - JSON file-based storage
  - Atomic writes for safe updates
  - Preset storage support
  - User-friendly formatting
- [x] Create `PresetManager.h/cpp` - manages preset load/save operations
  - Handles preset listing, loading, saving
  - Auto-save capability
- [x] Create `SettingsManager.h/cpp` - manages application settings
  - OSC server/client configuration
  - Window geometry and state
  - Input device preferences
- [x] Integrate into core library CMakeLists.txt

**Files Created:**
- `libs/sound2osc-core/include/sound2osc/config/ConfigStore.h`
- `libs/sound2osc-core/include/sound2osc/config/JsonConfigStore.h`
- `libs/sound2osc-core/src/config/JsonConfigStore.cpp`
- `libs/sound2osc-core/include/sound2osc/config/PresetManager.h`
- `libs/sound2osc-core/src/config/PresetManager.cpp`
- `libs/sound2osc-core/include/sound2osc/config/SettingsManager.h`
- `libs/sound2osc-core/src/config/SettingsManager.cpp`

#### Task 3: Headless Application Skeleton
- [x] Create `apps/headless/` directory structure
- [x] Implement `apps/headless/main.cpp` - basic CLI application
- [x] Create `apps/headless/CMakeLists.txt` - build configuration
- [x] Fix compilation errors and build successfully
- [x] Update root `CMakeLists.txt` to include `apps/headless` subdirectory
- [x] Test headless application functionality

**Files Created:**
- `apps/headless/main.cpp`
- `apps/headless/CMakeLists.txt`

**Build Configuration:**
- Enable with `-DSOUND2OSC_BUILD_HEADLESS=ON`
- Built executable: `build/bin/sound2osc-headless` (418KB)

#### Future Work (Deferred to Phase 4+)
The following items were originally in Phase 3 but are deferred as they are not blocking:

- [ ] Extract business logic from `MainController` to core library
   - Current: `MainController` is 1196 LOC (412 header, 784 source)
   - Core library has Qt dependencies (QObject) - acceptable for signal/slot usage
   - The new config system (PresetManager, SettingsManager) provides the foundation
- [ ] Update `MainController` to use new configuration system
   - Migrate from `QSettings` (ini format) to `JsonConfigStore`
   - Use `PresetManager` for preset operations
   - Use `SettingsManager` for application settings
- [ ] Make organization/app name configurable
   - Currently: Hard-coded strings in UI and config
   - SettingsManager provides the infrastructure for this
- [ ] Define clean interfaces between core and GUI
   - Core library uses Qt for signals (QObject/Q_OBJECT)
   - No GUI widgets (QWidget) in core - Clean separation maintained ✓
- [ ] Make audio backend abstraction complete (for future backends)
   - Currently: `AudioInputInterface` + `QAudioInputWrapper` (Qt implementation)
   - Status: Abstraction exists, single implementation

**Deliverables (Completed):**
- ✓ Cross-platform logging system with structured output
- ✓ JSON-based configuration backend with ConfigStore abstraction
- ✓ Preset management system with auto-save capability
- ✓ Application settings manager
- ✓ Working headless prototype (CLI application)
- ✓ Updated root CMakeLists.txt with headless build option (`-DSOUND2OSC_BUILD_HEADLESS=ON`)
- ✓ Both GUI (899KB) and headless (418KB) applications build and run successfully

### Phase 4: Technical Debt, Documentation & CI/CD

**Status:** In Progress (Updated 2026-01-19)

#### Task 1: Configuration System Migration
- [x] Create `AppInfo` class for configurable branding
- [x] Create `SettingsMigration` utility for legacy settings migration
- [x] Migrate `MainController` to use `SettingsManager`/`PresetManager`
- [x] Update `main.cpp` to use new configuration system

**Files Created:**
- `libs/sound2osc-core/include/sound2osc/core/AppInfo.h`
- `libs/sound2osc-core/src/core/AppInfo.cpp`
- `apps/gui/src/utils/SettingsMigration.h`
- `apps/gui/src/utils/SettingsMigration.cpp`

#### Task 2: Unit Tests
- [x] Create `TestLogger` unit tests
- [x] Create `TestConfigStore` unit tests
- [x] Create `TestAppInfo` unit tests
- [x] Update `tests/CMakeLists.txt` with new tests
- [x] Verify all 4 tests pass

**Test Status:** 4/4 tests passing (TestOSCMessage, TestLogger, TestConfigStore, TestAppInfo)

#### Task 3: Documentation
- [x] Create `docs/USER_GUIDE.md` - comprehensive user documentation
- [x] Create `docs/CONFIG_REFERENCE.md` - configuration options reference
- [x] Create `docs/OSC_REFERENCE.md` - OSC protocol documentation
- [x] Create `docs/TROUBLESHOOTING.md` - troubleshooting guide
- [x] Create `docs/building/LINUX.md` - Linux build instructions
- [x] Create `docs/building/WINDOWS.md` - Windows build instructions
- [x] Create `docs/building/MACOS.md` - macOS build instructions

#### Task 4: CI/CD & Packaging (In Progress)
- [x] Set up GitHub Actions workflow
- [ ] Test and fix Windows build
- [ ] Test and fix macOS build
- [ ] Create AppImage for Linux
- [ ] Create DMG for macOS
- [ ] Create Windows installer (NSIS or WiX)
- [ ] Automated release pipeline

**Deliverables:**
- ✓ Configurable branding with AppInfo class
- ✓ Seamless legacy settings migration
- ✓ Comprehensive unit test suite (4 tests)
- ✓ Complete user and developer documentation
- ⏳ Multi-platform CI/CD (in progress)
- ⏳ Automated release artifacts (in progress)

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
