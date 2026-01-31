# sound2osc Modernization Plan

This document outlines the remaining steps to achieve a fully modern, robust, and extensible architecture for sound2osc. The primary goal is to maintain a strict separation between the core signal processing logic and various frontend implementations while ensuring high code quality and testability.

---

## 1. Quality Assurance & Testing

The current test suite covers basic infrastructure but needs more depth to ensure the reliability of the core DSP algorithms.

- [ ] **DSP Verification** (`tests/unit/TestDSP.cpp`)
    - Implement tests for `FFTAnalyzer` using known synthetic signals (sine, square, white noise).
    - Verify that frequency bin alignment and energy calculation match mathematical expectations.
- [ ] **Trigger Logic Depth** (`tests/unit/TestTrigger.cpp`)
    - Validate `TriggerGenerator` behavior across edge cases (e.g., extremely low/high thresholds).
    - Specifically test time-domain filtering: on/off delays, hysteresis, and cooldown behaviors.
- [ ] **BPM Detection Stability** (`tests/unit/TestBPM.cpp`)
    - Test `BPMDetector` with various constant and shifting tempo signals.
    - Verify accuracy across a wide range of musical genres and beat patterns.
- [ ] **Integration Testing**
    - Create a suite in `tests/integration/` that validates the full pipeline: `AudioBuffer -> FFT -> Trigger -> OSC`.
    - Ensure the system remains stable during long-running headless operations.

---

## 2. Modern C++ & Code Quality

Refining the codebase to leverage C++17/20 features and improve memory safety.

- [ ] **Filesystem Migration**
    - Replace legacy `QDir` and `QFile` usage with `std::filesystem` where appropriate to reduce dependency on Qt's I/O layer in the core library.
- [ ] **Smart Pointer Migration**
    - Eliminate remaining raw `new`/`delete` calls.
    - Review `MainController` and GUI controllers to ensure proper use of `std::unique_ptr` and `std::shared_ptr`.
- [ ] **Const Correctness & Thread Safety**
    - Audit the hot paths (FFT/Trigger loops) for unnecessary copies and ensure thread-safe access to shared data buffers.

---

## 3. Portability & Performance

Increasing the flexibility of the core library for diverse deployment scenarios.

- [ ] **Alternative Audio Backends**
    - Implement non-Qt audio capture backends (e.g., pure PulseAudio, ALSA, or Miniaudio) to allow the headless application to run with minimal dependencies.
- [ ] **Headless Optimization**
    - Further optimize the `Sound2OscEngine` for low-latency operation on resource-constrained devices (e.g., Raspberry Pi Zero).

---

## 4. Advanced Architecture

Future-proofing the application for new use cases.

- [ ] **Web UI Backend**
    - Implement a WebSocket server within the core library to serve real-time spectrum and trigger data to a browser-based frontend (React/Vue).
- [ ] **Plugin System**
    - Architect a system to load custom trigger algorithms via shared libraries/DLLs, allowing users to extend the analysis capabilities without recompiling the core.
- [ ] **OSC Input Mapping**
    - Expand the remote control capabilities to allow full runtime configuration of all engine parameters via incoming OSC messages.
