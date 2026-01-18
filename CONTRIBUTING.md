# Contributing to Sound2OSC

Thank you for your interest in contributing to Sound2OSC! This guide explains the architecture, design principles, and workflow for extending the codebase.

---

## 1. Architecture Overview

### High-Level Design

Sound2OSC follows a **modular, layered architecture** with clear separation of concerns:

```
┌─────────────────────────────────────────────┐
│         UI Layer (QML / Qt Quick)           │
│  (Optional - Headless app has no GUI)       │
└─────────────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────┐
│    Application Layer (Controllers)          │
│  - MainController (orchestration)           │
│  - TriggerGuiController (UI binding)        │
│  - OSCMapping (network integration)         │
└─────────────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────┐
│  Core Library (sound2osc-core)              │
│  UI-Independent Business Logic              │
│                                             │
│  ┌─────────────────────────────────────┐  │
│  │  Audio     │ DSP     │ Trigger      │  │
│  │  ─────────────────────────────────  │  │
│  │  BPM       │ OSC     │ Config       │  │
│  │  ─────────────────────────────────  │  │
│  │  Logging   │ Utilities              │  │
│  └─────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────┐
│    External Dependencies (Qt6, FFFT)        │
└─────────────────────────────────────────────┘
```

**Key Principle:** The core library has zero UI dependencies. It communicates via Qt signals/slots (event system), not direct UI calls. This enables:
- Headless deployment (CLI/daemon)
- Future web-UI (reuse same core)
- Easy testing with mock objects

### Eight Core Components

| Component | Purpose | Key Classes | When to Modify |
|-----------|---------|------------|-----------------|
| **Audio** | Capture live audio from devices | `AudioInputInterface`, `QAudioInputWrapper`, `MonoAudioBuffer` | Adding new audio backends (ALSA, PulseAudio) |
| **DSP** | Real-time FFT analysis & spectrum processing | `FFTAnalyzer`, `ScaledSpectrum`, `FFTRealWrapper` | Optimizing FFT, adding spectral effects |
| **Trigger** | Detect frequency band level changes & generate OSC events | `TriggerGenerator`, `TriggerFilter`, `TriggerOscParameters` | New trigger types, timing algorithms |
| **BPM** | Beat detection & tempo analysis | `BPMDetector`, `BPMTapDetector`, `BPMOscControler` | Beat detection algorithms, tap input |
| **OSC** | Open Sound Control network protocol | `OSCNetworkManager`, `OSCMessage`, `OSCParser` | Protocol features, new transports |
| **Config** | Persistent settings & preset management | `IConfigStore`, `JsonConfigStore`, `PresetManager`, `SettingsManager` | New config backends (TOML, XML) |
| **Logging** | Cross-platform structured logging | `Logger` | Log output targets, formatting |
| **Core** | Shared utilities & data structures | `QCircularBuffer`, `utils.h`, `versionInfo.h` | Common helpers |

---

## 2. Component Dependency Rules

**IMPORTANT:** Maintain strict dependency boundaries. A component at layer N can depend on layers N-1 (below) but NOT on higher layers.

### Allowed Dependencies

```
┌────────────────────────────────────────┐
│ Application (GUI/Headless)             │ Depends on: Core + All components
├────────────────────────────────────────┤
│ Audio   │ DSP   │ Trigger              │ Depend on: Core, Logging, Config
├────────────────────────────────────────┤
│ BPM     │ OSC   │ Config               │ Depend on: Core, Logging
├────────────────────────────────────────┤
│ Logging │ Core (Utilities)             │ Depend on: Qt only (no internal deps)
└────────────────────────────────────────┘
```

### Violation Examples (DON'T DO THIS)

```cpp
// ❌ BAD: OSC component using Trigger directly (circular dependency)
class OSCMessage {
    TriggerGenerator* trigger;  // WRONG!
};

// ❌ BAD: Core using DSP (core should not depend on specialized logic)
namespace sound2osc {
    FFTAnalyzer* globalFFT;  // WRONG!
}

// ✓ GOOD: Core uses only Qt and standard library
namespace sound2osc {
    QVector<float> buffer;  // OK
    std::unique_ptr<float> data;  // OK
}
```

---

## 3. Core Library Design Principles

### 3.1 Interface-Based Extensibility

Define abstract interfaces for every pluggable component:

**Example: Audio Input**
```cpp
// In header: AudioInputInterface.h
// - Abstract virtual methods (getAvailableInputs, setInputByName, etc.)
// - No implementation
// - Optional: Pure virtual destructor

// In implementation: QAudioInputWrapper.cpp
// - Concrete implementation using Qt Multimedia
// - Constructor registers with factory if needed

// In future: ALSAInputWrapper.cpp
// - Alternative ALSA implementation
// - Same interface, different internals
```

**Benefit:** Swap implementations without changing calling code.

### 3.2 Composition Over Inheritance

Prefer aggregation to class hierarchies:

```cpp
// ✓ GOOD: Composition
class FFTAnalyzer {
    std::unique_ptr<BasicFFTInterface> m_fft;  // Composed object
    MonoAudioBuffer& m_buffer;
    QVector<TriggerGeneratorInterface*> m_triggers;
};

// ❌ AVOID: Deep inheritance hierarchies
class AdvancedFFTAnalyzer : public FFTAnalyzer {
    class OptimizedFFTAnalyzer : public AdvancedFFTAnalyzer {
        // Three levels deep - hard to extend
    };
};
```

### 3.3 RAII (Resource Acquisition Is Initialization)

Manage resources through object lifetimes:

```cpp
// ✓ GOOD: Constructor acquires, destructor releases
class OSCNetworkManager {
    QUdpSocket m_udpSocket;  // Created/destroyed with manager
    void initialize() {
        m_udpSocket.bind(QHostAddress::Any, m_rxPort);
    }
};

// ❌ AVOID: Manual acquire/release
class BadNetworking {
    QUdpSocket* m_socket = nullptr;
    void init() {
        m_socket = new QUdpSocket();  // Must remember to delete
    }
    ~BadNetworking() {
        if (m_socket) delete m_socket;  // Easy to forget
    }
};
```

### 3.4 Signal/Slot Pattern (Observer)

Use Qt signals for inter-component communication:

```cpp
// ✓ GOOD: Loose coupling via signals
class TriggerFilter : public QObject {
    signals:
        void onSignalSent();    // Emitted when trigger activates
        void offSignalSent();   // Emitted when trigger deactivates
};

// In MainController:
connect(triggerFilter, &TriggerFilter::onSignalSent,
        oscManager, &OSCNetworkManager::sendMessage);

// ❌ AVOID: Direct function calls (tight coupling)
class TriggerFilterBad {
    OSCNetworkManager* m_osc;  // Tight coupling
    void sendTrigger() {
        m_osc->sendMessage("/trigger", "1");  // Direct call
    }
};
```

---

## 4. Adding New Features

### 4.1 Adding a New Trigger Type

**Goal:** Create a trigger that fires based on spectral centroid (color of sound).

**Steps:**

1. **Create header** `libs/sound2osc-core/include/sound2osc/trigger/CentroidTrigger.h`
   - Inherit from `TriggerGeneratorInterface`
   - Implement required pure virtuals: `getMidFreq()`, `setMidFreq()`, etc.
   - Add new properties: `getCentroidSensitivity()`, `setCentroidSensitivity()`

2. **Create implementation** `libs/sound2osc-core/src/trigger/CentroidTrigger.cpp`
   - Calculate spectral centroid from `ScaledSpectrum`
   - Implement `checkForTrigger()` to compare centroid against threshold
   - Use `TriggerFilter` for timing (on-delay, off-delay, hold)

3. **Update CMakeLists.txt** `libs/sound2osc-core/CMakeLists.txt`
   - Add `src/trigger/CentroidTrigger.cpp` to source list

4. **Update GUI** (if needed) `apps/gui/src/controllers/TriggerGuiController.cpp`
   - Add UI binding for `getCentroidSensitivity()` property
   - Connect QML slider to `setCentroidSensitivity()`

5. **Add tests** (future) `tests/unit/TriggerCentroid_test.cpp`
   - Mock `ScaledSpectrum` with known data
   - Verify centroid calculation
   - Verify threshold crossing logic

**Do NOT:**
- Call GUI functions from trigger code
- Create QWidget or QML objects in the core library
- Access global state directly

### 4.2 Adding a New OSC Message Type

**Goal:** Send amplitude envelopes (RMS over 100ms windows).

**Steps:**

1. **Extend `OSCMessage`** or create `EnvelopeOSCMessage`
   - Handle new argument types if needed

2. **Update `OSCNetworkManager`** to support queuing envelopes
   - New method: `sendEnvelope(const QString& path, const QVector<float>& envelope)`
   - Serialize envelope as OSC array

3. **Connect in `MainController`**
   - Create timer for 100ms windows
   - Calculate RMS from `MonoAudioBuffer`
   - Call `oscManager->sendEnvelope("/spectrum/envelope", rmsValues)`

4. **Test transmission** (manual or automated)
   - Verify OSC packets are valid (use `oscvalidate` tool if available)

### 4.3 Adding Configuration Options

**Goal:** Allow users to select FFT window function (Hann, Blackman, etc.).

**Steps:**

1. **Update `FFTAnalyzer`**
   - Add property: `getWindowFunction()` / `setWindowFunction(WindowType)`
   - Enum: `enum class WindowType { Hann, Blackman, Hamming }`

2. **Update `ScaledSpectrum`** or FFT wrapper
   - Apply window function before FFT calculation

3. **Save to config** via `SettingsManager`
   - New key: `"dsp/windowFunction"` = `"Hann"`

4. **Load in `MainController`**
   ```cpp
   QString window = settings->value("dsp/windowFunction", "Hann");
   fftAnalyzer->setWindowFunction(stringToWindowType(window));
   ```

5. **Expose in GUI** (optional)
   - ComboBox in settings panel bound to `MainController::windowFunctionIndex`

---

## 5. Real-Time Considerations

Audio processing runs at 44 Hz in the main GUI event loop. **DO NOT:**

- Allocate memory in hot loop (`calculateFFT()`, `checkForTrigger()`)
- Call blocking I/O (file reads, network operations)
- Resize vectors or reallocate buffers
- Print debug statements (use `Logger` asynchronously instead)

**Pre-allocate all buffers** in constructors:

```cpp
// ✓ GOOD: Pre-allocate once
class FFTAnalyzer {
    QVector<float> m_spectrum;
    QVector<float> m_window;
    
    FFTAnalyzer() {
        m_spectrum.resize(200);  // Pre-allocate
        m_window.resize(4096);   // Pre-allocate
    }
    
    void calculateFFT() {
        // No allocation here - just compute
        std::copy(m_buffer.begin(), m_buffer.end(), m_spectrum.begin());
    }
};

// ❌ BAD: Allocates in hot loop
void BadCalculateFFT() {
    QVector<float> temp(4096);  // WRONG - allocates every 22ms!
}
```

---

## 6. Testing & Verification

### 6.1 Manual Testing Checklist

Before submitting a change:

- [ ] Compiles without warnings (`clang-tidy` compliant)
- [ ] Headless app runs: `./build/bin/sound2osc-headless --list-devices`
- [ ] GUI app runs: `./build/bin/sound2osc` displays spectrum
- [ ] Audio input works: Sing/play music, see spectrum move
- [ ] Triggers fire: Check OSC monitor for outgoing messages
- [ ] Settings save/load: Restart app, settings persist
- [ ] Cross-platform (if available): Test Linux, macOS, Windows

### 6.2 Recommended Logging

Add diagnostic logging for troubleshooting:

```cpp
#include <sound2osc/logging/Logger.h>

void TriggerGenerator::checkForTrigger(ScaledSpectrum& spectrum, bool forceRelease) {
    float currentLevel = spectrum.getMaxLevel(m_midFreq, m_width);
    
    if (currentLevel > m_threshold) {
        Logger::debug("Trigger '%1' fired: level=%.2f threshold=%.2f", 
                     m_name, currentLevel, m_threshold);
        m_filter.triggerOn();
    }
}
```

Use log levels appropriately:
- **Debug:** Detailed algorithm flow (threshold crossing, FFT values)
- **Info:** State changes (device switched, trigger created)
- **Warning:** Recoverable issues (audio underrun, OSC dropped packets)
- **Error:** Non-fatal issues (config file not found, OSC connection failed)
- **Critical:** Fatal issues (insufficient memory, core dump imminent)

### 6.3 Unit Test Structure (Future)

When unit tests are implemented:

```cpp
// tests/unit/FFTAnalyzer_test.cpp

#include <catch2/catch.hpp>
#include <sound2osc/dsp/FFTAnalyzer.h>

TEST_CASE("FFTAnalyzer calculates correct spectrum") {
    MockFFT mockFft;  // Mock BasicFFTInterface
    MockBuffer mockBuffer;  // Mock MonoAudioBuffer
    FFTAnalyzer analyzer(mockBuffer, {});
    
    // Set up known input (e.g., 440 Hz sine wave)
    mockBuffer.setPeakFrequency(440);
    
    analyzer.calculateFFT(false);
    
    // Verify spectrum peak near 440 Hz bin
    REQUIRE(analyzer.getScaledSpectrum().getMaxLevel(440, 0.1) > 0.8);
}
```

---

## 7. Code Style & Conventions

### 7.1 Naming

| Entity | Convention | Example |
|--------|-----------|---------|
| Classes | PascalCase | `FFTAnalyzer`, `TriggerGenerator` |
| Methods | camelCase | `calculateFFT()`, `getMidFreq()` |
| Properties | camelCase getter/setter | `getMidFreq()`, `setMidFreq(int)` |
| Constants | SCREAMING_SNAKE_CASE | `NUM_SAMPLES`, `DEFAULT_OSC_PORT` |
| Member vars | m_ prefix + camelCase | `m_buffer`, `m_fft`, `m_triggers` |
| Namespaces | lowercase | `namespace sound2osc` |
| Enums | PascalCase (type) + SCREAMING_SNAKE_CASE (values) | `enum class Level { Debug, Info, Warning }` |

### 7.2 Header Guards

Use `#ifndef` with specific guard naming:

```cpp
#ifndef SOUND2OSC_DSP_FFTANALYZER_H
#define SOUND2OSC_DSP_FFTANALYZER_H

// ... content ...

#endif // SOUND2OSC_DSP_FFTANALYZER_H
```

### 7.3 Includes

Organize includes in order:

```cpp
// 1. Own header
#include <sound2osc/dsp/FFTAnalyzer.h>

// 2. sound2osc headers
#include <sound2osc/dsp/BasicFFTInterface.h>
#include <sound2osc/logging/Logger.h>

// 3. Qt headers
#include <QVector>
#include <QObject>

// 4. Standard library
#include <vector>
#include <memory>

// 5. Third-party
#include <ffft/FFTRealFixLen.h>
```

### 7.4 const-Correctness

Mark methods `const` if they don't modify state:

```cpp
// ✓ GOOD
class ScaledSpectrum {
    float getLevelAtFreq(int freq) const;  // Read-only
    void setGain(float gain);               // Modifies state
};

// ✘ Avoid
float getMaxLevel(int freq) const {  // Should be const
    m_cache = calculateMax();  // ERROR - modifying in const method
}
```

---

## 8. Git Workflow

### 8.1 Commit Message Format

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat:` New feature (e.g., `feat(trigger): add centroid trigger type`)
- `fix:` Bug fix (e.g., `fix(osc): handle invalid path format`)
- `refactor:` Code restructuring (e.g., `refactor(dsp): simplify spectrum calculation`)
- `perf:` Performance improvement (e.g., `perf(fft): reduce memory allocations`)
- `docs:` Documentation (e.g., `docs: update CONTRIBUTING.md`)
- `test:` Test additions (e.g., `test(bpm): add onset detection test`)
- `chore:` Build system, dependencies (e.g., `chore: update Qt6 to 6.9`)

**Example:**

```
feat(config): add TOML backend support

- Implement TomlConfigStore inheriting from IConfigStore
- Add toml library to CMakeLists.txt dependencies
- Update SettingsManager to auto-detect config format

This enables TOML format presets alongside JSON.

Closes #42
```

### 8.2 Creating a Branch

```bash
# From main branch
git checkout -b feat/centroid-trigger
# or
git checkout -b fix/osc-message-overflow

# Push when ready
git push -u origin feat/centroid-trigger
```

### 8.3 Before Submitting

```bash
# Build and test
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Check for compiler warnings
# (output goes to build/compile_commands.json for IDE inspection)

# Run headless app test
./build/bin/sound2osc-headless --help

# Run GUI test
./build/bin/sound2osc &
# (manually verify spectrum display and trigger firing)

# Commit
git add .
git commit -m "feat(trigger): add centroid trigger type"

# Push
git push origin feat/centroid-trigger
```

---

## 9. Performance Profiling

### 9.1 Identifying Bottlenecks

For real-time critical functions:

```bash
# Linux: Use perf (if available)
perf record ./build/bin/sound2osc
perf report

# macOS: Use Instruments
xcrun xctrace record --template "System Trace" ./build/bin/sound2osc

# Or use Qt Creator's built-in profiler:
# Tools → Analyzer → QML Profiler
```

### 9.2 Common Hotspots

These run at 44+ Hz - optimize aggressively:

1. **FFTAnalyzer::calculateFFT()** - FFT computation (~22ms budget)
2. **TriggerGenerator::checkForTrigger()** - Threshold detection (6 triggers × 22ms)
3. **ScaledSpectrum::updateWithLinearSpectrum()** - Spectrum scaling (22ms)
4. **BPMDetector::detectBPM()** - Beat detection (20 Hz, less critical)

**Optimization strategies:**
- Pre-allocate buffers (never new/delete in loop)
- Use SIMD (if supported by compiler)
- Cache frequency-to-bin lookups
- Reduce function call overhead (use inline, templates)

---

## 10. Documentation

### 10.1 Header Comments

Every public class/function should have documentation:

```cpp
/**
 * @brief Real-time FFT analyzer with trigger orchestration
 * 
 * Reads audio samples from MonoAudioBuffer, performs FFT analysis,
 * and checks all registered triggers against the resulting spectrum.
 * Runs at ~44 Hz from main GUI event loop.
 * 
 * @see BasicFFTInterface for FFT implementation details
 * @see TriggerGeneratorInterface for trigger contract
 */
class FFTAnalyzer {
    /**
     * Perform FFT analysis and check triggers
     * 
     * Reads buffered audio samples, computes FFT, applies spectral
     * processing (normalization, gain, compression), and triggers
     * each registered trigger detector.
     * 
     * @param lowSoloMode If true, ignore spectral data above ~1kHz
     * @note Must be called from audio callback or event loop
     * @note Real-time safe (no allocations)
     */
    void calculateFFT(bool lowSoloMode);
};
```

### 10.2 Inline Comments

For complex algorithms, explain the "why" not the "what":

```cpp
// ❌ DON'T
x = sqrt(spectrum[i]);  // Take square root

// ✓ DO
// Apply sqrt to compress dynamic range - makes spectrum more visible
// on linear scale while preserving relative magnitudes
x = sqrt(spectrum[i]);

// ✓ BETTER (for really complex logic)
// Centroid calculated as: sum(freq_i * magnitude_i) / sum(magnitude_i)
// This gives the "center of mass" frequency of the spectrum.
// See: https://en.wikipedia.org/wiki/Spectral_centroid
float centroid = calculateCentroid(spectrum);
```

---

## 11. Troubleshooting

### 11.1 Common Build Issues

| Error | Cause | Fix |
|-------|-------|-----|
| `Qt6::Core not found` | Qt6 not installed | `sudo apt install qt6-base-dev` |
| `undefined reference to '_ZN6sound2osc...` | CMakeLists.txt missing source file | Add `.cpp` file to `add_executable()` |
| `No such file: qml6-module-qtquick` | Missing Qt6 QML modules | `sudo apt install qml6-module-qt*` |
| `fatal error: sound2osc/audio/AudioInput.h` | Include path wrong | Check `target_include_directories()` in CMakeLists.txt |

### 11.2 Common Runtime Issues

| Symptom | Cause | Fix |
|---------|-------|-----|
| "No audio devices found" | Audio subsystem not initialized | Restart PulseAudio: `systemctl --user restart pulseaudio` |
| "OSC packets not sending" | Firewall blocking UDP | `sudo ufw allow 9000/udp` |
| "Spectrum doesn't move" | Audio input at 0 volume | Check system mixer / app volume slider |
| "Settings not saving" | Config directory doesn't exist | App auto-creates `~/.config/sound2osc/` |

### 11.3 Debug Logging

Enable debug logging to diagnose issues:

```bash
# Run with verbose logging
QT_LOGGING_RULES="*=true" ./build/bin/sound2osc-headless --verbose --list-devices
```

Or programmatically:

```cpp
Logger::initialize("sound2osc", Logger::Output::Console | Logger::Output::File);
Logger::setLogLevel(Logger::Level::Debug);
Logger::setLogFile("/tmp/sound2osc.log");
```

---

## 12. Getting Help

- **Architecture questions:** Review MODERNIZATION_PLAN.md and this file
- **API documentation:** Check header file comments in `libs/sound2osc-core/include/sound2osc/`
- **Build issues:** Check `cmake/` directory and CMakeLists.txt files
- **Real-time concerns:** Review section 5 (Real-Time Considerations)
- **Code examples:** See section 10 of CODEBASE_ANALYSIS.md

---

## Summary

When contributing, remember:

1. **Core library** is UI-independent; use it from any frontend
2. **Signal/slot pattern** replaces direct function calls
3. **Pre-allocate** everything in constructors; never allocate in hot loops
4. **Interface-based** design enables easy testing and future extensions
5. **Composition** over inheritance for modularity
6. **RAII** for resource management
7. **Commit messages** follow Conventional Commits format
8. **Performance** matters - 44 Hz real-time constraint is tight
9. **Logging** for diagnostics, not printing to console
10. **Documentation** in headers, not in comments

Welcome aboard! 🚀
