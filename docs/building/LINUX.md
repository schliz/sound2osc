# Building sound2osc on Linux

This guide covers building sound2osc from source on Linux distributions.

## Supported Distributions

- Debian 12 (Bookworm) / 13 (Trixie)
- Ubuntu 22.04 LTS / 24.04 LTS
- Fedora 38+
- Arch Linux

## Prerequisites

### Debian / Ubuntu

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
    qt6-quickcontrols2-dev \
    libgl1-mesa-dev \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-dialogs \
    qml6-module-qtquick-window \
    qml6-module-qtmultimedia
```

### Fedora

```bash
sudo dnf install -y \
    cmake \
    ninja-build \
    git \
    gcc-c++ \
    qt6-qtbase-devel \
    qt6-qtmultimedia-devel \
    qt6-qtdeclarative-devel \
    qt6-qtquickcontrols2-devel \
    mesa-libGL-devel
```

### Arch Linux

```bash
sudo pacman -S --needed \
    base-devel \
    cmake \
    ninja \
    git \
    qt6-base \
    qt6-multimedia \
    qt6-declarative \
    qt6-quickcontrols2
```

## Clone the Repository

```bash
git clone https://github.com/your-org/sound2osc.git
cd sound2osc
```

## Build Configuration

### Release Build (Recommended)

```bash
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DSOUND2OSC_BUILD_HEADLESS=ON
```

### Debug Build

```bash
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSOUND2OSC_BUILD_HEADLESS=ON
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug/Release/RelWithDebInfo) |
| `SOUND2OSC_BUILD_HEADLESS` | OFF | Build the headless CLI application |
| `SOUND2OSC_BUILD_TESTS` | ON | Build unit tests |

## Compile

```bash
cmake --build build
```

For parallel builds (faster):

```bash
cmake --build build -- -j$(nproc)
```

## Run Tests

```bash
cd build
ctest --output-on-failure
```

## Install

### System-wide Installation

```bash
sudo cmake --install build
```

Default installation paths:
- Executables: `/usr/local/bin/`
- Libraries: `/usr/local/lib/`

### Custom Installation Prefix

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/sound2osc ...
cmake --build build
sudo cmake --install build
```

## Running

### GUI Application

```bash
./build/bin/sound2osc
```

### Headless Application

```bash
./build/bin/sound2osc-headless --list-devices
./build/bin/sound2osc-headless --help
```

## Troubleshooting

### Qt6 Not Found

If CMake can't find Qt6:

```bash
# Set Qt6 path explicitly
cmake -B build -DQt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6 ...
```

### QML Modules Not Found

Install the required QML modules:

```bash
# Debian/Ubuntu
sudo apt install qml6-module-qtquick-layouts qml6-module-qtquick-controls

# Check what's installed
dpkg -l | grep qml6
```

### Audio Device Permissions

If audio devices aren't accessible:

```bash
# Add user to audio group
sudo usermod -a -G audio $USER

# Log out and back in for changes to take effect
```

### PipeWire/PulseAudio Issues

Ensure audio server is running:

```bash
# Check PipeWire status
systemctl --user status pipewire pipewire-pulse

# Or PulseAudio
pulseaudio --check -v
```

## Creating Packages

### DEB Package (Debian/Ubuntu)

```bash
cd build
cpack -G DEB
```

### RPM Package (Fedora/RHEL)

```bash
cd build
cpack -G RPM
```

### AppImage

```bash
# Install linuxdeploy
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage

# Create AppImage
./linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --executable build/bin/sound2osc \
    --desktop-file resources/sound2osc.desktop \
    --icon-file resources/icons/sound2osc.png \
    --output appimage
```

## Running as a Service

Create a systemd service file:

```ini
# /etc/systemd/system/sound2osc.service
[Unit]
Description=sound2osc Audio Analysis Service
After=sound.target network.target

[Service]
Type=simple
User=sound2osc
ExecStart=/usr/local/bin/sound2osc-headless --config /etc/sound2osc/config.json
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable sound2osc
sudo systemctl start sound2osc
```

## IDE Setup

### Qt Creator

1. Open Qt Creator
2. File > Open File or Project
3. Select `CMakeLists.txt` in the root directory
4. Configure build settings
5. Build and run

### Visual Studio Code

Install extensions:
- C/C++ (Microsoft)
- CMake Tools

Open the project folder and CMake Tools will auto-detect the configuration.

### CLion

1. File > Open
2. Select the project root directory
3. CLion will auto-detect CMake configuration

## See Also

- [Building on Windows](WINDOWS.md)
- [Building on macOS](MACOS.md)
- [Troubleshooting](../TROUBLESHOOTING.md)
