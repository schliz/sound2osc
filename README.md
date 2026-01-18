> [!NOTE]
> This is a fork of the [Sound2Light Tool](https://github.com/ETCLabs/Sound2Light) by Electronic Theatre Controls, Inc.

# sound2osc

**Real-time audio analysis to OSC trigger events** for live entertainment systems.

The sound2osc tool converts live audio signals to trigger events that can be sent as OSC messages. It can reproduce the sound-to-light function of the NT/NTX consoles with systems of the Eos-, Cobalt- and ColorSource-family. It can also be remotely controlled by OSC.

## Features

### Audio Analysis
- **Real-time FFT spectrum analysis** – 44 Hz update rate with minimal latency
- **Beat detection** – Automatic BPM detection from audio input
- **Frequency-based triggers** – Configurable frequency bands for spectral analysis
- **Multiple detection algorithms** – Beat peaks, frequency thresholds, and custom triggers

### Network Integration
- **OSC Protocol** – Send trigger events over UDP to compatible devices
- **Remote control** – Receive OSC messages to adjust settings and presets
- **Cross-platform networking** – Works with Eos, Cobalt, ColorSource, and other OSC-compatible systems

### Deployment Options
- **GUI Application** – Interactive, real-time visualization with full configuration UI
- **Headless/CLI Application** – Lightweight daemon for server environments without X11 or display servers

### Configuration & Presets
- **JSON-based configuration** – Easy to version control and integrate with automation
- **Preset management** – Save and load audio analysis configurations
- **Real-time adjustments** – Modify settings without restarting

---

## Quick Start

### Installation

#### Prerequisites
- **Qt 6.2+** (with Multimedia module)
- **CMake 3.23+**
- **Ninja** (recommended) or Make
- **libfftw3** (FFT library)
- **C++17 compatible compiler** (GCC 7+, Clang 5+, MSVC 2017+)

#### Build from Source

```bash
# Clone repository
git clone <repository-url>
cd sound2osc

# Configure with CMake
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DSOUND2OSC_BUILD_HEADLESS=ON

# Build both GUI and headless applications
cmake --build build

# Verify build succeeded
./build/bin/sound2osc --version
./build/bin/sound2osc-headless --version
```

### Running the Application

#### GUI Application (Interactive)
```bash
./build/bin/sound2osc
```

Features:
- Visual spectrum analyzer
- Real-time waveform display
- Interactive trigger configuration
- OSC message monitoring
- Audio device selection

#### Headless Application (Server)
```bash
./build/bin/sound2osc-headless --help
./build/bin/sound2osc-headless --list-devices
./build/bin/sound2osc-headless --config config.json --verbose
```

Common options:
- `--list-devices` – Show available audio input devices
- `--device <name>` – Select audio device by name
- `--config <file>` – Load configuration from JSON file
- `--osc-host <host>` – OSC destination host (default: localhost)
- `--osc-port <port>` – OSC destination port (default: 9000)
- `--verbose` – Enable debug logging
- `--help` – Show all options
- `--version` – Display version information

---

## Documentation

### For Users
- **[User Guide](./docs/USER_GUIDE.md)** – How to use the GUI, configure presets, and integrate with lighting consoles
- **[Configuration Reference](./docs/CONFIG_REFERENCE.md)** – JSON configuration format and options
- **[OSC Message Reference](./docs/OSC_REFERENCE.md)** – Complete list of OSC messages and parameters
- **[Troubleshooting](./docs/TROUBLESHOOTING.md)** – Common issues and solutions

### For Developers
- **[Contributing Guide](./CONTRIBUTING.md)** – Architecture overview, design principles, and feature development workflow
- **[Modernization Plan](./MODERNIZATION_PLAN.md)** – Project roadmap, phase status, and architectural decisions
- **[AI Agent Guide](./AGENTS.md)** – Information for AI assistants and automated tools working on the codebase

### Architecture

#### Core Components
The application is built on a modular, UI-independent core library (`libs/sound2osc-core/`):

| Component | Purpose | Status |
|-----------|---------|--------|
| **Audio** | Cross-platform audio capture abstraction | ✅ Complete |
| **DSP** | Real-time FFT and spectrum analysis | ✅ Complete |
| **Trigger** | Trigger generation and detection algorithms | ✅ Complete |
| **BPM** | Beat detection and BPM estimation | ✅ Complete |
| **OSC** | Open Sound Control protocol implementation | ✅ Complete |
| **Config** | JSON-based configuration and presets | ✅ Complete |
| **Logging** | Cross-platform structured logging | ✅ Complete |
| **Core** | Utilities, buffers, and data structures | ✅ Complete |

#### Directory Structure
```
sound2osc/
├── libs/sound2osc-core/          Core library (UI-independent)
│   ├── include/sound2osc/        Public API headers
│   └── src/                      Implementation files
├── apps/
│   ├── gui/                      Qt GUI application
│   └── headless/                 CLI/daemon application
├── tests/                        Unit and integration tests
├── docs/                         User documentation
├── CONTRIBUTING.md               Developer guide
├── AGENTS.md                     AI assistant guide
├── MODERNIZATION_PLAN.md         Architecture roadmap
└── CMakeLists.txt               Root build configuration
```

---

## Building & Development

### Build Options

```bash
# GUI only (default)
cmake -B build -DSOUND2OSC_BUILD_HEADLESS=OFF

# Both GUI and headless
cmake -B build -DSOUND2OSC_BUILD_HEADLESS=ON

# Headless only
cmake -B build -DSOUND2OSC_BUILD_GUI=OFF -DSOUND2OSC_BUILD_HEADLESS=ON
```

### Build Types

```bash
# Release (optimized, ~880KB executable)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Debug (with symbols, larger executable)
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

### Development Workflow

See **[Contributing Guide](./CONTRIBUTING.md)** for:
- How to add new features
- Component interaction patterns
- Real-time constraints and performance considerations
- Code style and naming conventions
- Git workflow with conventional commits

---

## Project Status

### Current Phase: Phase 3 ✅ Complete
**Architecture Refactoring & Headless App**
- Core library fully refactored to Qt6 and C++17
- Headless application implemented and tested
- Both GUI and CLI apps working correctly
- Documentation completed

### Next Phase: Phase 4
**CI/CD & Cross-Platform Support**
- GitHub Actions workflow for multi-platform builds (Linux, Windows, macOS)
- Automated testing with CTest
- Release pipeline with installers (AppImage, DMG, NSIS)

See **[Modernization Plan](./MODERNIZATION_PLAN.md)** for complete roadmap and technical decisions.

---

## License

This project is based on the [Sound2Light Tool](https://github.com/ETCLabs/Sound2Light) by Electronic Theatre Controls, Inc.

---

## Contributing

Contributions are welcome! Please see:
1. **[Contributing Guide](./CONTRIBUTING.md)** – Start here for architecture and design principles
2. **[Modernization Plan](./MODERNIZATION_PLAN.md)** – Understand the current phase and roadmap
3. Git Workflow – Use [conventional commits](https://www.conventionalcommits.org/) for all commits

---

## Support

### Getting Help
- Check **[Troubleshooting Guide](./docs/TROUBLESHOOTING.md)** for common issues
- Review **[OSC Reference](./docs/OSC_REFERENCE.md)** for protocol details
- Read **[Configuration Reference](./docs/CONFIG_REFERENCE.md)** for JSON schema

### Reporting Issues
Please open an issue on GitHub with:
- Operating system and version
- Qt version (if built from source)
- Steps to reproduce
- Expected vs. actual behavior
- Configuration file (if applicable)

---

## Acknowledgments

- Based on [Sound2Light Tool](https://github.com/ETCLabs/Sound2Light) by ETC
- Built with Qt 6
- FFT powered by libfftw3
- OSC implementation based on liblo patterns
