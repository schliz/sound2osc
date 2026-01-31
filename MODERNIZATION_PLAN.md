# sound2osc Modernization Plan

**Status:** Phase 4 (Quality Assurance) & Phase 5 (Refactoring) in progress.
**Last Updated:** 2026-01-25

This document outlines the remaining steps to complete the modernization of sound2osc. Phases 1-3 (Qt6 migration, Core separation, Headless app) are complete.

---

## Phase 4: Quality Assurance & Testing (Current Priority)

The current test suite covers infrastructure (Config, Logging, OSC) but lacks coverage for the core DSP and logic components.

### 4.1 Expand Unit Test Coverage
- [ ] **DSP Testing** (`tests/unit/TestDSP.cpp`)
    - Test `FFTAnalyzer` with known signal inputs (sine waves).
    - Verify spectrum output matches expected frequencies.
- [ ] **Trigger Logic** (`tests/unit/TestTrigger.cpp`)
    - Test `TriggerGenerator` threshold logic.
    - Test hysteresis and cooldown behaviors.
- [ ] **BPM Detection** (`tests/unit/TestBPM.cpp`)
    - Test `BPMDetector` with constant tempo signals.
- [ ] **Audio Abstraction**
    - Ensure `AudioInputInterface` behaves correctly with mock inputs.

### 4.2 Integration Testing
- [ ] Create `tests/integration/` suite.
- [ ] Verify full pipeline (AudioBuffer -> FFT -> Trigger -> OSC) in a headless environment.

---

## Phase 5: Architecture Refactoring (Critical)

Currently, the business logic (wiring Audio to FFT to Triggers) is duplicated between `apps/gui/src/controllers/MainController.cpp` and `apps/headless/main.cpp`.

### 5.1 Extract Core Engine
- [x] **Create `Sound2OscEngine` class** in `libs/sound2osc-core`.
    - Move wiring logic from `MainController` and `headless/main.cpp` into this class.
    - Should own `AudioInput`, `FFTAnalyzer`, `TriggerGenerator`s, and `BPMDetector`.
    - Should handle the main processing loop/timers.
- [x] **Refactor Headless App**
    - Replace raw component instantiation in `headless/main.cpp` with `Sound2OscEngine`.
- [x] **Refactor GUI App**
    - Update `MainController` to wrap `Sound2OscEngine` instead of managing individual components.
    - `MainController` becomes strictly a bridge between QML and the Engine.

### 5.2 Complete Configuration Migration
- [x] **Finalize Settings Migration**
    - Ensure `MainController` exclusively uses `SettingsManager` and `JsonConfigStore`.
    - Remove any lingering `QSettings` usage for business logic.
- [x] **Clean up TODOs**
    - Restore view positions/geometry in GUI (currently marked as TODO).
    - Restore trigger settings visibility (marked as TODO).

---

## Phase 6: Future & Polish

### 6.1 Modern C++ Standards
- [ ] Replace `QDir`/`QFile` with `std::filesystem` where appropriate (C++17).
- [ ] Review raw pointer usage in `MainController` (replace with `std::unique_ptr`/`std::shared_ptr`).

### 6.2 Advanced Features
- [ ] **Web UI Backend:** Implement WebSocket server in `apps/web-ui` (or core) to serve a React/Vue frontend.
- [ ] **Audio Backends:** Implement non-Qt audio backends (e.g., pure ALSA or PulseAudio) for lighter headless deployments.
- [ ] **Plugin System:** Allow custom trigger algorithms via shared libraries.

---

## Completed Milestones (Reference)

- **Phase 1:** Repository Cleanup & Build System (CMake/Ninja) ✅
- **Phase 2:** Qt6 & C++17 Migration ✅
- **Phase 3:** Architecture Refactoring (Headless App, Logger, ConfigStore) ✅
- **CI/CD:** GitHub Actions for Linux/Windows/macOS ✅
