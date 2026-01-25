# Building sound2osc on Windows

This guide covers building sound2osc from source on Windows.

## Prerequisites

### Required Software

1. **Visual Studio 2022** (or 2019)
   - Download from: https://visualstudio.microsoft.com/
   - Install "Desktop development with C++" workload

2. **CMake 3.23+**
   - Download from: https://cmake.org/download/
   - Add to PATH during installation

3. **Qt 6.5+**
   - Download Qt Online Installer: https://www.qt.io/download-open-source
   - Install Qt 6.x for MSVC 2019/2022 64-bit
   - Components needed:
     - Qt 6.x / MSVC 2022 64-bit
     - Qt 6.x / Qt Multimedia
     - Qt 6.x / Qt Quick / Qt Quick Controls

4. **Git**
   - Download from: https://git-scm.com/download/win

### Optional

- **Ninja** (faster builds): https://ninja-build.org/
- **vcpkg** (alternative package manager): https://vcpkg.io/

## Clone the Repository

Open Command Prompt or PowerShell:

```powershell
git clone https://github.com/your-org/sound2osc.git
cd sound2osc
```

## Build Configuration

### Using Visual Studio Generator

```powershell
# Set Qt path (adjust version as needed)
$env:Qt6_DIR = "C:\Qt\6.6.0\msvc2019_64\lib\cmake\Qt6"

# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_BUILD_TYPE=Release `
    -DSOUND2OSC_BUILD_HEADLESS=ON
```

### Using Ninja (Faster)

```powershell
# Open "x64 Native Tools Command Prompt for VS 2022"
# Or run vcvarsall.bat first

set Qt6_DIR=C:\Qt\6.6.0\msvc2019_64\lib\cmake\Qt6

cmake -B build -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DSOUND2OSC_BUILD_HEADLESS=ON
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug/Release) |
| `SOUND2OSC_BUILD_HEADLESS` | OFF | Build CLI application |
| `SOUND2OSC_BUILD_TESTS` | ON | Build unit tests |

## Compile

### Command Line

```powershell
cmake --build build --config Release
```

### Visual Studio

1. Open `build/sound2osc.sln`
2. Set configuration to Release
3. Build > Build Solution (F7)

## Run Tests

```powershell
cd build
ctest -C Release --output-on-failure
```

## Running

The executables are in `build\bin\Release\`:

```powershell
# GUI Application
.\build\bin\Release\sound2osc.exe

# Headless Application
.\build\bin\Release\sound2osc-headless.exe --list-devices
```

### Deploying Qt Dependencies

Before distributing, copy Qt DLLs:

```powershell
# Use windeployqt
cd build\bin\Release
C:\Qt\6.6.0\msvc2019_64\bin\windeployqt.exe --qmldir ..\..\..\apps\gui\qml sound2osc.exe
```

## Installation

### Manual Installation

Copy the following to your installation directory:
- `sound2osc.exe`
- `sound2osc-headless.exe`
- All DLLs from windeployqt
- `qml/` folder

### Creating an Installer

#### Using NSIS

1. Install NSIS: https://nsis.sourceforge.io/
2. Run CPack:

```powershell
cd build
cpack -G NSIS
```

#### Using WiX Toolset

1. Install WiX: https://wixtoolset.org/
2. Run CPack:

```powershell
cd build
cpack -G WIX
```

## Troubleshooting

### Qt Not Found

Set the Qt6_DIR environment variable:

```powershell
# PowerShell
$env:Qt6_DIR = "C:\Qt\6.6.0\msvc2019_64\lib\cmake\Qt6"

# Command Prompt
set Qt6_DIR=C:\Qt\6.6.0\msvc2019_64\lib\cmake\Qt6

# Or add to CMake command
cmake -B build -DQt6_DIR="C:\Qt\6.6.0\msvc2019_64\lib\cmake\Qt6" ...
```

### MSVC Not Found

Run CMake from a Visual Studio Developer Command Prompt, or run vcvarsall.bat first:

```powershell
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

### Missing DLLs at Runtime

1. Run windeployqt (see above)
2. Or add Qt bin directory to PATH:

```powershell
$env:PATH += ";C:\Qt\6.6.0\msvc2019_64\bin"
```

### Audio Device Not Found

1. Check Windows Sound Settings
2. Ensure device isn't exclusively used by another app
3. Run as Administrator if needed

### Firewall Blocking OSC

1. Windows Defender Firewall > Allow an app through firewall
2. Add `sound2osc.exe` and `sound2osc-headless.exe`
3. Enable for Private and Public networks if needed

## Alternative: Using vcpkg

vcpkg can manage Qt dependencies:

```powershell
# Install vcpkg
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Install Qt6 (takes a while)
.\vcpkg\vcpkg install qt6:x64-windows

# Configure with vcpkg toolchain
cmake -B build -G Ninja ^
    -DCMAKE_TOOLCHAIN_FILE=path\to\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DSOUND2OSC_BUILD_HEADLESS=ON
```

## IDE Setup

### Visual Studio

1. File > Open > Folder
2. Select the project root
3. Visual Studio will auto-detect CMake
4. Select startup item: sound2osc.exe

### Visual Studio Code

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools
2. Open the project folder
3. CMake Tools will detect the configuration
4. Select kit: Visual Studio 2022 Release - amd64

### Qt Creator

1. Open Qt Creator
2. File > Open File or Project
3. Select `CMakeLists.txt`
4. Configure build kit (MSVC 2022 64-bit)
5. Build and run

## See Also

- [Building on Linux](LINUX.md)
- [Building on macOS](MACOS.md)
- [Troubleshooting](../TROUBLESHOOTING.md)
