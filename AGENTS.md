# AI Agent Instructions

> **Project Status: Phase 5.2 Complete ✅**
>
> The sound2osc project has completed Phase 5.2 (Configuration Migration). Presets are now stored as JSON files, and the `Sound2OscEngine` manages the entire application state. See [MODERNIZATION_PLAN.md](./MODERNIZATION_PLAN.md) for remaining steps.

## Overview for AI Assistants

This document provides structured information for AI agents and automated tools working on the sound2osc codebase. It supplements the human-facing [Contributing Guide](./CONTRIBUTING.md) with technical constraints, architectural rules, and code patterns.

### Project Essentials

- **Project Type:** Qt6 C++17 audio analysis and OSC protocol application
- **Architecture:** Modular core library + GUI/CLI frontends
- **Build System:** CMake 3.23+
- **Target:** Linux, Windows, macOS (Phase 4: CI/CD in progress)
- **Status:** Feature-complete, Phase 3 done, Phase 4 starting

---

## 1. Critical Architectural Rules

These rules are INVIOLABLE. Violations break the entire architecture:

### 1.1 Core Library Independence (CRITICAL)
```
✅ ALLOWED in /libs/sound2osc-core/:
  - Qt6::Core, Qt6::Network, Qt6::Multimedia
  - Standard library (C++17)
  - Third-party libraries (libfftw3)
  
❌ FORBIDDEN in /libs/sound2osc-core/:
  - Qt6::Gui, Qt6::Quick, Qt6::Widgets
  - Any UI-related classes or functions
  - Direct file I/O (use Logger for logging)
  - GUI-specific configuration (e.g., QSettings)

RATIONALE: Core library must work headless, in server environments, and with future web UIs.
```

**Violation Detection:** If any file in `/libs/sound2osc-core/` includes headers from Qt GUI modules, it's a violation.

### 1.2 Real-Time Constraints (CRITICAL)
```
✅ ALLOWED in FFTAnalyzer, TriggerGenerator (hot path):
  - Pre-allocated buffers (reuse existing allocations)
  - Stack allocation for small data
  - Inline functions
  - Zero-copy operations

❌ FORBIDDEN in hot path (44 Hz audio loop):
  - Dynamic memory allocation (malloc, new, std::make_unique)
  - std::vector push_back, resize, or other allocations
  - Blocking operations (file I/O, network calls)
  - Signal emissions (use direct calls instead)
  - Exception handling in critical sections

RATIONALE: Audio arrives at 44 Hz (every 23ms). Memory allocation can exceed this timeout.
```

**How to identify hot paths:**
- `FFTAnalyzer::analyze()` - processes audio buffer
- `TriggerGenerator::detect()- checks for triggers
- `BPMDetector::analyze()` - beat detection
- Audio callback functions in AudioCapture

### 1.3 Signal/Slot Pattern (ARCHITECTURAL)
```
✅ REQUIRED for inter-component communication:
  - Components communicate via Qt signals/slots
  - Example: AudioCapture signals → FFTAnalyzer processes
  - Enables loose coupling, easy testing, async operation

❌ FORBIDDEN:
  - Direct function calls between components
  - Passing component pointers for calls
  - Component A calling Component B methods directly

PATTERN:
  class AudioCapture : public QObject {
      Q_SIGNALS:
          void audioDataReady(const AudioBuffer& data);
  };
  
  class FFTAnalyzer : public QObject {
      Q_SLOTS:
          void onAudioData(const AudioBuffer& data) { analyze(data); }
  };
  
  // In MainController:
  connect(audioCapture, &AudioCapture::audioDataReady,
          fftAnalyzer, &FFTAnalyzer::onAudioData);
```

### 1.4 Component Dependency Graph (FIXED)
```
ALLOWED dependencies (DAG - no cycles):
  AudioCapture → FFTAnalyzer → TriggerGenerator → OSCClient
                → BPMDetector → [reports to MainController]
  
  Sound2OscEngine → [Owns all above components]
  MainController → Sound2OscEngine
  
  All components → Logger (for logging)
  All components → Config (read-only for settings)

❌ FORBIDDEN cycles:
  - AudioCapture depends on anything that depends on it
  - FFTAnalyzer depends on TriggerGenerator that depends on it
  - Any backward dependencies
```

---

## 2. Navigation Reference

Quick file location guide:

### Public API Headers (Don't break these)
```
libs/sound2osc-core/include/sound2osc/
├── audio/            [AudioCapture, AudioBuffer, AudioDevice]
├── dsp/              [FFTAnalyzer, Spectrum, SpectrumBuffer]
├── trigger/          [TriggerGenerator, TriggerEvent]
├── bpm/              [BPMDetector, BeatInfo]
├── osc/              [OSCClient, OSCMessage]
├── config/           [JsonConfigStore, PresetManager]
├── logging/          [Logger]
└── core/             [Utilities, buffers, enums]
```

### Implementation Files (Safe to modify)
```
libs/sound2osc-core/src/
├── AudioCapture.cpp          [Audio input, device enumeration]
├── FFTAnalyzer.cpp           [Real-time FFT, spectrum analysis]
├── TriggerGenerator.cpp       [Trigger detection algorithms]
├── BPMDetector.cpp           [Beat detection]
├── OSCClient.cpp             [OSC network messages]
├── JsonConfigStore.cpp       [Config persistence]
├── Logger.cpp                [Cross-platform logging]
└── [other .cpp files]
```

### Application Code
```
apps/gui/                      [Qt GUI application]
├── main.cpp                  [Entry point]
├── MainWindow.cpp            [Main UI window]
└── [Qt UI components]

apps/headless/                 [CLI/daemon application]
├── main.cpp                  [Entry point, CLI parsing]
└── [minimal UI code]
```

### Configuration & Build
```
CMakeLists.txt                 [Root build config]
MODERNIZATION_PLAN.md          [Architecture decisions, roadmap]
CONTRIBUTING.md                [Human-facing developer guide]
README.md                      [User-facing overview]
```

---

## 3. Common Development Tasks

### Task: Add a New Trigger Type

**Steps:**
1. Add enum to `include/sound2osc/trigger/TriggerType.h`
2. Implement detection in `src/TriggerGenerator.cpp`
3. Add JSON schema support in `src/JsonConfigStore.cpp`
4. Add UI control in `apps/gui/TriggerConfigWidget.cpp`
5. Test in headless: `./build/bin/sound2osc-headless --help`
6. Commit: `feat(trigger): Add <trigger-type> detection`

**Files to modify:**
- `include/sound2osc/trigger/TriggerType.h` (public API)
- `src/TriggerGenerator.cpp` (implementation)
- `src/JsonConfigStore.cpp` (persistence)
- `apps/gui/TriggerConfigWidget.cpp` (UI)

**Constraints:**
- No memory allocation in `detect()` hot path
- Use signal/slots for updates to GUI
- Update both GUI and headless in same commit

### Task: Modify FFT Parameters

**Files:**
- `include/sound2osc/dsp/FFTAnalyzer.h` (parameters)
- `src/FFTAnalyzer.cpp` (implementation)
- `include/sound2osc/dsp/Spectrum.h` (output format)

**Constraints:**
- Pre-allocate buffers in constructor
- Don't allocate in `analyze()` method
- Update Logger calls if changing behavior
- Verify headless app works: `./build/bin/sound2osc-headless --list-devices`

### Task: Add OSC Message Parameter

**Files:**
- `include/sound2osc/osc/OSCMessage.h` (message structure)
- `src/OSCClient.cpp` (sending implementation)
- `src/JsonConfigStore.cpp` (config schema)
- `apps/gui/OSCConfigWidget.cpp` (UI)

**Constraints:**
- OSC addresses must be valid (e.g., `/sound2osc/spectrum`)
- Must support both numeric and string payloads
- Log all sent messages when verbose mode enabled
- Test with headless first

### Task: Fix a Crash in Audio Capture

**Investigation:**
1. Run headless app with verbose: `./build/bin/sound2osc-headless --verbose`
2. Check logs for error messages
3. Look at `src/AudioCapture.cpp` for the issue
4. Check `src/Logger.cpp` for logging patterns

**Common issues:**
- Device not found: Check device list with `--list-devices`
- Format mismatch: Check audio format compatibility
- Buffer overflow: Check buffer sizes in `AudioBuffer.h`

---

## 4. Code Patterns & Idioms

### Pattern: Logging

```cpp
#include "sound2osc/logging/Logger.h"

void SomeComponent::doSomething() {
    Logger::info("Starting process");
    Logger::debug("Parameter value: " + QString::number(param));
    Logger::warning("Unexpected condition detected");
    Logger::error("Failed to open device");
}
```

**Rules:**
- Use `Logger::*` in core library, not `std::cout` or `qDebug()`
- Use `Logger::debug()` in hot paths only with verbose flag
- Log at construction and destruction of major objects

### Pattern: Component Initialization

```cpp
class MyComponent : public QObject {
public:
    MyComponent(const std::shared_ptr<ConfigStore>& config, QObject* parent = nullptr)
        : QObject(parent), m_config(config) {
        
        Logger::info("MyComponent initialized");
        
        // Pre-allocate buffers
        m_buffer.resize(1024);
        m_data.reserve(512);
    }

private:
    std::shared_ptr<ConfigStore> m_config;
    QVector<float> m_buffer;      // Pre-allocated
    std::vector<float> m_data;    // Capacity reserved
};
```

**Rules:**
- Use `std::shared_ptr` for component references
- Pre-allocate all buffers in constructor
- Don't allocate in hot paths
- Initialize member variables in member initializer list

### Pattern: Signal/Slot Connection

```cpp
// In MainController:
connect(audioCapture, &AudioCapture::audioDataReady,
        fftAnalyzer, &FFTAnalyzer::onAudioData,
        Qt::QueuedConnection);  // Cross-thread safe

connect(oscClient, &OSCClient::messageSent,
        logger, &Logger::onOSCMessageSent,
        Qt::DirectConnection);  // Same thread
```

**Rules:**
- Use `Qt::QueuedConnection` for thread safety
- Use `Qt::DirectConnection` for same-thread calls
- Always use `&ClassName::method` syntax
- Connect in MainController, not in components

### Pattern: Configuration Access

```cpp
#include "sound2osc/config/JsonConfigStore.h"

void MyComponent::applyConfig(const std::shared_ptr<JsonConfigStore>& config) {
    // Reading config
    int sampleRate = config->getInt("audio", "sampleRate", 44100);
    QString device = config->getString("audio", "device", "default");
    
    // Saving config
    config->setInt("audio", "sampleRate", 48000);
    config->save();
    
    Logger::info("Config applied");
}
```

**Rules:**
- Read config in constructor or initialization
- Save config after user changes
- Use default values (3rd parameter)
- Use correct data type (getInt, getString, getDouble, etc.)

---

## 5. Build & Verification

### Build Commands

```bash
# Clean build
cd /home/user/Dokumente2/sound2osc
rm -rf build
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DSOUND2OSC_BUILD_HEADLESS=ON
cmake --build build

# Incremental build
cmake --build build

# Build specific target
cmake --build build --target sound2osc-headless
cmake --build build --target sound2osc
```

### Verification Checklist

After any code change:
```
□ Build succeeds: cmake --build build
□ No new compiler errors in sound2osc-core
□ GUI app launches: ./build/bin/sound2osc &
□ Headless works: ./build/bin/sound2osc-headless --list-devices
□ Logger output is sensible: ./build/bin/sound2osc-headless --verbose
```

### Common Build Issues

| Issue | Solution |
|-------|----------|
| `error: unknown type name 'AudioCapture'` | Missing `#include "sound2osc/audio/AudioCapture.h"` |
| `error: cannot initialize a variable of type 'int'` | Type mismatch in function return or assignment |
| `undefined reference to 'symbol'` | Missing `target_link_libraries()` in CMakeLists.txt |
| `Qt6::Gui not found` | Trying to use GUI modules in core library (violation!) |

---

## 6. Architecture Constraints

### Memory Management
- **Core Library:** Use `std::shared_ptr`, `std::unique_ptr`, or Qt `QObject` parent system
- **Hot Paths:** Pre-allocate, never allocate in tight loops
- **GUI:** Qt handles memory through parent-child relationships

### Thread Safety
- **Audio Callbacks:** Run on audio thread, use `Qt::QueuedConnection` to signal
- **Network:** OSC client runs on separate thread, thread-safe
- **UI:** All Qt operations on main thread via signals/slots

### Error Handling
- **Core Library:** Use return codes or exceptions carefully
- **Logging:** Always log errors with `Logger::error()`
- **Headless:** Exit with non-zero code on critical errors
- **GUI:** Show user-friendly error messages via QMessageBox

---

## 7. Modification Checklist

Before committing code:

### Code Quality
- [ ] No raw `new`/`delete` in new code
- [ ] No `#include <Qt/QGui>` in core library
- [ ] All memory allocations documented
- [ ] Hot paths don't allocate memory
- [ ] Logger used instead of `std::cout`

### Testing
- [ ] Build succeeds with no errors
- [ ] GUI app launches and doesn't crash
- [ ] Headless app responds to `--help` and `--list-devices`
- [ ] No new compiler warnings in sound2osc targets

### Documentation
- [ ] Public API changes documented in headers
- [ ] Complex logic commented with "why" not "what"
- [ ] New components added to CONTRIBUTING.md component list
- [ ] Breaking changes added to MODERNIZATION_PLAN.md

### Git Workflow
- [ ] Changes follow Conventional Commits (feat:, fix:, refactor:, docs:, etc.)
- [ ] Commit message is clear and concise
- [ ] One logical change per commit
- [ ] No merge commits (rebase if needed)

---

## 8. Known Constraints & Workarounds

### Qt6 Migration
- All code must be Qt6-compatible
- Don't use deprecated Qt5 APIs
- Qt6::Multimedia for audio (replaces Qt5::Multimedia)
- Signals/slots syntax uses `&ClassName::method` (Qt5-compatible)

### Cross-Platform Issues
- **Audio:** Use Qt6::Multimedia for cross-platform audio
- **File Paths:** Use `QDir::separator()` not hardcoded `/` or `\`
- **Line Endings:** Git handles this, but be aware
- **Thread IDs:** Use `std::this_thread` not platform-specific code

### Performance
- FFT runs at 44 Hz, any slower = audible lag
- BPM detection needs ~2 seconds of audio
- Spectrum updates 44 times per second
- OSC messages sent every frame (1 per 23ms)

---

## 9. Escalation & Getting Unstuck

### Before Escalating
1. Check relevant CONTRIBUTING.md section
2. Look at similar code in codebase (find patterns)
3. Build verbosely: `cmake --build build --verbose`
4. Check git history for similar changes: `git log --oneline --grep="<pattern>"`

### When Stuck
- **Architecture question:** Review MODERNIZATION_PLAN.md Phase 3 decisions
- **API question:** Read header files in `include/sound2osc/`
- **Build error:** Check CMakeLists.txt in component directory
- **Runtime error:** Use `--verbose` flag in headless app for detailed logs

### Escalation Path
1. Read error message carefully (often has the answer)
2. Search codebase for similar code
3. Check Contributing Guide for relevant pattern
4. Report issue with:
   - Exact error message
   - Steps to reproduce
   - What you've already tried
   - Relevant code snippets

---

## 10. Commit Conventions

Use [Conventional Commits](https://www.conventionalcommits.org/):

```
feat(audio):  Add new audio device filter
fix(trigger): Correct threshold calculation
refactor(dsp): Simplify FFT initialization
docs(readme): Update build instructions
style(code): Format FFTAnalyzer.cpp
test(unit): Add FFTAnalyzer unit tests
ci(github): Configure GitHub Actions
```

### Commit Rules
- **Type:** feat, fix, refactor, docs, style, test, ci
- **Scope:** Component name (audio, dsp, trigger, osc, etc.)
- **Subject:** Lowercase, no period, imperative mood
- **Body:** Explain *why* not *what*
- **Footer:** References issues: `Fixes #123`

---

## 11. Quick Reference Commands

### Build & Test
```bash
cmake -B build -G Ninja -DSOUND2OSC_BUILD_HEADLESS=ON
cmake --build build                                    # Build all
cmake --build build --target sound2osc-headless       # Build one

./build/bin/sound2osc                                 # Run GUI
./build/bin/sound2osc-headless --list-devices         # List audio devices
./build/bin/sound2osc-headless --help                 # Show CLI options
./build/bin/sound2osc-headless --verbose              # Debug logging
```

### Git
```bash
git add [files]                                        # Stage changes
git commit -m "feat(component): Add feature"           # Commit
git log --oneline -10                                 # Recent commits
git diff HEAD~1                                        # See last change
git status                                             # Working tree status
```

### File Navigation
```bash
find /home/user/Dokumente2/sound2osc -name "*.h"      # Find headers
rg "class AudioCapture" libs/sound2osc-core/          # Search code
grep -r "Q_SIGNALS" libs/sound2osc-core/include/     # Find patterns
```

---

## 12. Reference Information

### Core Components Summary
| Component | Purpose | Hot Path | Qt Modules |
|-----------|---------|----------|-----------|
| Sound2OscEngine | Central orchestration | No | Core |
| AudioCapture | Audio input abstraction | No | Multimedia |
| FFTAnalyzer | Spectrum analysis | **YES** | Core |
| TriggerGenerator | Trigger detection | **YES** | Core |
| BPMDetector | Beat detection | **YES** | Core |
| OSCClient | Network messages | No | Network |
| JsonConfigStore | Config/presets | No | Core |
| Logger | Logging system | No | Core |

### Phase Status
- **Phase 1:** ✅ Core library architecture (complete)
- **Phase 2:** ✅ GUI application (complete)
- **Phase 3:** ✅ Headless app & refactoring (complete)
- **Phase 4:** ⏳ CI/CD & cross-platform (starting)

### Build Output
- GUI executable: `build/bin/sound2osc` (~880KB)
- Headless executable: `build/bin/sound2osc-headless` (~412KB)
- CMake config: `build/CMakeFiles/` and `build/CMakeCache.txt`

---

## Document Maintenance

This document is for AI assistants. For human developers, see:
- **[CONTRIBUTING.md](./CONTRIBUTING.md)** – Full architecture guide and development workflow
- **[MODERNIZATION_PLAN.md](./MODERNIZATION_PLAN.md)** – Project roadmap and decisions
- **[README.md](./README.md)** – User guide and feature overview

Last updated: Phase 3 completion (January 2026)
