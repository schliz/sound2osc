# Building sound2osc on Debian 13/14

This guide provides step-by-step instructions for building the sound2osc application on Debian 13 (trixie) or Debian 14 (testing) using the modern CMake build system.

Note: The project was previously named Sound2Light or "s2l" for short.

## Prerequisites

A Debian 13/14 installation with:
- Internet connection
- `sudo` privileges
- Basic system utilities (`apt`, `git`)

## Build Dependencies

sound2osc is a Qt6-based C++ application. Install the required dependencies:

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-declarative-dev \
    libqt6quick6 \
    libqt6quickcontrols2-6 \
    qt6-quickcontrols2-dev \
    libgl1-mesa-dev \
    qml6-module-qtquick \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-window \
    qml6-module-qtquick-dialogs \
    qml6-module-qtmultimedia
```

### Package Breakdown

- **build-essential**: GCC compiler, make, and essential build tools
- **cmake**: Modern build system generator
- **ninja-build**: Fast build system (optional but recommended)
- **qt6-base-dev**: Qt6 core development files
- **qt6-multimedia-dev**: Qt6 Multimedia module for audio input handling
- **qt6-declarative-dev**: Qt6 QML/QtQuick development files
- **qt6-quickcontrols2-dev**: Qt Quick Controls 2 development files
- **libgl1-mesa-dev**: OpenGL development files (required for Qt Quick)
- **qml6-module-*** packages: Runtime QML modules required by the application

## Building the Application

### 1. Navigate to Source Directory

```bash
cd sound2osc
```

### 2. Configure with CMake

Use CMake to configure the build with Ninja:

```bash
cmake -B build -G Ninja
```

Or use the default Make generator:

```bash
cmake -B build
```

### 3. Build the Application

```bash
cmake --build build
```

Or for parallel builds with Make:

```bash
cmake --build build -- -j$(nproc)
```

**Expected Output**: The executable `sound2osc` will be created in `build/bin/`.

### 4. Verify Build

Check that the executable was created:

```bash
ls -lh build/bin/sound2osc
```

## Running the Application

```bash
./build/bin/sound2osc
```

### Expected Behavior

On first run, you should see:
- "this is the first start of the software, nothing to restore"
- Some QML warnings (these are harmless during migration)
- TCP connection errors if no OSC receivers are configured (this is normal)

The GUI application window should appear with the sound2osc interface.

## Build Options

The following CMake options are available:

| Option | Default | Description |
|--------|---------|-------------|
| `SOUND2OSC_BUILD_GUI` | ON | Build the Qt6 GUI application |
| `SOUND2OSC_BUILD_HEADLESS` | OFF | Build the headless CLI (future) |
| `SOUND2OSC_BUILD_TESTS` | OFF | Build unit tests |

Example with options:

```bash
cmake -B build -G Ninja -DSOUND2OSC_BUILD_TESTS=ON
```

## Troubleshooting

### Issue: "Could not find Qt6"

**Solution**: Install Qt6 development packages:
```bash
sudo apt install qt6-base-dev
```

### Issue: QML module not found

**Solution**: Install missing QML runtime modules:
```bash
sudo apt install qml6-module-qtquick-controls qml6-module-qtquick-dialogs
```

### Issue: No audio devices detected

**Solution**: Ensure PulseAudio/PipeWire is running and your user has access to audio devices:
```bash
sudo usermod -a -G audio $USER
```
Log out and log back in for group changes to take effect.

### Issue: OpenGL errors

**Solution**: Install Mesa OpenGL development files:
```bash
sudo apt install libgl1-mesa-dev
```

## Clean Build

To perform a clean rebuild:

```bash
rm -rf build
cmake -B build -G Ninja
cmake --build build
```

## Development

For development, you may want to use a Debug build:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The `compile_commands.json` file is automatically generated for IDE integration.

## System Requirements

- **OS**: Debian 13/14 (or compatible Linux distribution with Qt6)
- **CPU**: Any modern x86_64 processor
- **RAM**: Minimum 512 MB, 1 GB recommended
- **Disk Space**: ~100 MB for dependencies, ~50 MB for build
- **Audio**: Any PipeWire, PulseAudio, or ALSA compatible audio input device
- **Graphics**: OpenGL 2.0 capable graphics (for Qt Quick)

## Project Structure

```
sound2osc/
├── CMakeLists.txt          # Root CMake configuration
├── cmake/                  # CMake modules
├── libs/
│   └── sound2osc-core/    # Core library (audio, OSC, DSP)
├── apps/
│   └── gui/               # Qt6 GUI application
├── third_party/
│   └── ffft/              # FFT library
└── docs/                  # Documentation
```

See [MODERNIZATION_PLAN.md](./MODERNIZATION_PLAN.md) for details on the project structure.
