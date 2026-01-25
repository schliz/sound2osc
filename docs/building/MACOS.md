# Building sound2osc on macOS

This guide covers building sound2osc from source on macOS.

## Supported Versions

- macOS 12 (Monterey) and later
- Apple Silicon (M1/M2/M3) and Intel x86_64

## Prerequisites

### Xcode Command Line Tools

```bash
xcode-select --install
```

### Homebrew

Install Homebrew if not already installed:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Required Packages

```bash
brew install cmake ninja qt@6
```

### Set Qt Path

Add to your shell profile (`~/.zshrc` or `~/.bash_profile`):

```bash
export Qt6_DIR=$(brew --prefix qt@6)/lib/cmake/Qt6
export PATH="$(brew --prefix qt@6)/bin:$PATH"
```

Reload your shell:

```bash
source ~/.zshrc  # or source ~/.bash_profile
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

### Universal Binary (Intel + Apple Silicon)

```bash
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DSOUND2OSC_BUILD_HEADLESS=ON
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug/Release) |
| `SOUND2OSC_BUILD_HEADLESS` | OFF | Build CLI application |
| `SOUND2OSC_BUILD_TESTS` | ON | Build unit tests |
| `CMAKE_OSX_ARCHITECTURES` | native | Target architectures |

## Compile

```bash
cmake --build build
```

For parallel builds:

```bash
cmake --build build -- -j$(sysctl -n hw.ncpu)
```

## Run Tests

```bash
cd build
ctest --output-on-failure
```

## Running

### GUI Application

```bash
./build/bin/sound2osc.app/Contents/MacOS/sound2osc

# Or open as app
open ./build/bin/sound2osc.app
```

### Headless Application

```bash
./build/bin/sound2osc-headless --list-devices
./build/bin/sound2osc-headless --help
```

## Creating a Distributable App Bundle

### Deploy Qt Dependencies

```bash
# Navigate to the app
cd build/bin

# Use macdeployqt
$(brew --prefix qt@6)/bin/macdeployqt sound2osc.app \
    -qmldir=../../apps/gui/qml \
    -always-overwrite
```

### Code Signing (For Distribution)

```bash
# Sign the app
codesign --force --deep --sign "Developer ID Application: Your Name" sound2osc.app

# Verify
codesign --verify --verbose sound2osc.app
```

### Notarization (For Distribution Outside App Store)

```bash
# Create a zip for notarization
ditto -c -k --keepParent sound2osc.app sound2osc.zip

# Submit for notarization
xcrun notarytool submit sound2osc.zip \
    --apple-id "your@email.com" \
    --team-id "TEAM_ID" \
    --password "app-specific-password" \
    --wait

# Staple the notarization ticket
xcrun stapler staple sound2osc.app
```

## Creating a DMG

```bash
# Install create-dmg
brew install create-dmg

# Create DMG
create-dmg \
    --volname "sound2osc" \
    --volicon "resources/icons/sound2osc.icns" \
    --window-pos 200 120 \
    --window-size 600 400 \
    --icon-size 100 \
    --icon "sound2osc.app" 150 190 \
    --hide-extension "sound2osc.app" \
    --app-drop-link 450 190 \
    "sound2osc-1.0.0.dmg" \
    "build/bin/"
```

Or use CPack:

```bash
cd build
cpack -G DragNDrop
```

## Installation

### Manual Installation

1. Drag `sound2osc.app` to `/Applications`
2. For headless: Copy `sound2osc-headless` to `/usr/local/bin`

### Homebrew Cask (Future)

```bash
brew install --cask sound2osc
```

## Troubleshooting

### Qt Not Found

Ensure Qt path is set:

```bash
export Qt6_DIR=$(brew --prefix qt@6)/lib/cmake/Qt6

# Verify
echo $Qt6_DIR
ls $Qt6_DIR
```

### "App is damaged" Error

Clear the quarantine attribute:

```bash
xattr -cr /path/to/sound2osc.app
```

### Gatekeeper Warning

For development builds:

```bash
# Allow apps from anywhere (not recommended for production)
sudo spctl --master-disable

# Or remove quarantine from specific app
xattr -d com.apple.quarantine /path/to/sound2osc.app
```

### Microphone Permission Denied

1. Open System Preferences > Security & Privacy > Privacy
2. Select Microphone
3. Enable access for sound2osc (or Terminal if running from command line)

### No Audio Devices Listed

Check system permissions:
1. System Preferences > Security & Privacy > Privacy
2. Ensure Input Monitoring includes Terminal/sound2osc

### Architecture Mismatch

If you get architecture errors:

```bash
# Check current architecture
uname -m

# Build for specific architecture
cmake -B build -DCMAKE_OSX_ARCHITECTURES=arm64 ...  # M1/M2/M3
cmake -B build -DCMAKE_OSX_ARCHITECTURES=x86_64 ... # Intel
```

## Running as a Service (launchd)

Create a launch daemon:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.sound2osc.headless</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/sound2osc-headless</string>
        <string>--config</string>
        <string>/etc/sound2osc/config.json</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
    <key>StandardOutPath</key>
    <string>/var/log/sound2osc.log</string>
    <key>StandardErrorPath</key>
    <string>/var/log/sound2osc.log</string>
</dict>
</plist>
```

Save as `/Library/LaunchDaemons/com.sound2osc.headless.plist` and:

```bash
sudo launchctl load /Library/LaunchDaemons/com.sound2osc.headless.plist
```

## IDE Setup

### Xcode

```bash
# Generate Xcode project
cmake -B build -G Xcode \
    -DSOUND2OSC_BUILD_HEADLESS=ON

# Open in Xcode
open build/sound2osc.xcodeproj
```

### Qt Creator

1. Open Qt Creator
2. File > Open File or Project
3. Select `CMakeLists.txt`
4. Configure build kit
5. Build and run

### Visual Studio Code

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools
2. Open the project folder
3. Select kit: Clang
4. Configure and build

### CLion

1. File > Open
2. Select the project root
3. CLion auto-detects CMake
4. Configure build settings

## See Also

- [Building on Linux](LINUX.md)
- [Building on Windows](WINDOWS.md)
- [Troubleshooting](../TROUBLESHOOTING.md)
