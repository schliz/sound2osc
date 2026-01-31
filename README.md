# sound2osc

**Real-time audio analysis to OSC trigger events.**

sound2osc converts live audio signals into Open Sound Control (OSC) messages for lighting consoles, visualizers, and interactive systems. It provides real-time FFT spectrum analysis, beat detection, and configurable frequency triggers.

## Features

- **Real-time Analysis**: 44Hz update rate with minimal latency.
- **Trigger System**: Configurable frequency bands, level thresholds, and beat detection.
- **OSC Integration**: Send triggers to Eos, Cobalt, Resolume, or any OSC-enabled software.
- **Dual Modes**: 
  - **GUI**: Interactive visualization and configuration.
  - **Headless**: Lightweight CLI daemon for servers and embedded devices.
- **Cross-Platform**: Runs on Linux, Windows, and macOS.

## Installation

Download the latest pre-compiled binary for your platform from the [Releases](https://github.com/ETCLabs/sound2osc/releases) page.

## Usage

### GUI Application
Launch the application to open the configuration interface:
```bash
./sound2osc
```

### Headless (CLI)
Run without a graphical interface for server environments:
```bash
./sound2osc-headless --device "default" --osc-host 192.168.1.100
```

Common options:
- `--list-devices`: Show available audio inputs.
- `--config <file>`: Load a JSON configuration file.
- `--verbose`: Enable detailed logging.

## Documentation

- [User Guide](docs/USER_GUIDE.md)
- [OSC Reference](docs/OSC_REFERENCE.md)
- [Configuration Reference](docs/CONFIG_REFERENCE.md)

## Development

This project uses CMake and Qt 6.

### Requirements
- C++17 compatible compiler
- CMake 3.23+
- Qt 6.2+ (Core, Network, Multimedia)
- libfftw3

### Build
```bash
cmake -B build
cmake --build build
```

For detailed development guidelines, see [CONTRIBUTING.md](CONTRIBUTING.md).

## License

Based on the Sound2Light Tool by Electronic Theatre Controls, Inc.
