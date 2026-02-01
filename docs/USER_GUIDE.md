# sound2osc User Guide

sound2osc is a real-time audio analysis application that converts audio signals into OSC (Open Sound Control) messages. It's designed for live performances, lighting control, and interactive installations.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Main Interface](#main-interface)
3. [Audio Input](#audio-input)
4. [Spectrum Display](#spectrum-display)
5. [Triggers](#triggers)
6. [BPM Detection](#bpm-detection)
7. [OSC Configuration](#osc-configuration)
8. [Presets](#presets)
9. [Headless Mode](#headless-mode)

---

## Getting Started

### Launching the Application

**GUI Mode:**
```bash
./sound2osc
```

**Headless Mode (CLI):**
```bash
./sound2osc-headless --help
```

### First-Time Setup

1. Connect your audio input device (microphone, audio interface, etc.)
2. Launch sound2osc
3. Select your audio input device from the dropdown
4. Configure your OSC destination (IP address and port)
5. Start monitoring and adjust trigger thresholds

---

## Main Interface

The main window consists of several sections:

### Top Bar
- **Preset selector**: Load and save trigger configurations
- **Audio input selector**: Choose your audio input device
- **Volume meter**: Visual feedback of input level

### Center Area
- **Spectrum display**: Real-time frequency visualization
- **Trigger indicators**: Visual feedback when triggers fire

### Bottom Panel
- **BPM display**: Current detected tempo
- **OSC status**: Connection status and message count

---

## Audio Input

### Selecting an Input Device

1. Click the audio input dropdown
2. Select your desired input device
3. The spectrum display should start showing audio activity

### Input Level

- Monitor the input level meter to ensure adequate signal
- Adjust your audio interface gain if the signal is too low or clipping
- Ideal level: peaks reaching 70-80% of the meter

### Supported Formats

- Sample rates: 44100 Hz, 48000 Hz
- Channels: Mono (stereo is mixed down)
- Bit depth: 16-bit, 24-bit, 32-bit float

---

## Spectrum Display

The spectrum display shows real-time frequency analysis of the audio input.

### Frequency Bands

The spectrum is divided into frequency bands from low (bass) to high (treble):

| Band | Frequency Range | Typical Sources |
|------|-----------------|-----------------|
| Sub Bass | 20-60 Hz | Kick drums, bass drops |
| Bass | 60-250 Hz | Bass guitar, toms |
| Low Mids | 250-500 Hz | Vocals, guitars |
| Mids | 500-2000 Hz | Vocals, instruments |
| High Mids | 2000-4000 Hz | Clarity, presence |
| Highs | 4000-20000 Hz | Cymbals, air, sparkle |

### Display Options

- **Linear/Log scale**: Toggle between linear and logarithmic frequency display
- **Peak hold**: Show peak levels with decay
- **Smoothing**: Adjust response time of the display

---

## Triggers

Triggers are the core feature of sound2osc. They detect specific audio characteristics and send OSC messages when conditions are met.

### Trigger Types

1. **Level Trigger**: Fires when overall level exceeds threshold
2. **Band Trigger**: Fires when a specific frequency band exceeds threshold
3. **Onset Trigger**: Fires on sudden transients (drum hits, etc.)
4. **BPM Trigger**: Fires on detected beats

### Configuring Triggers

1. Select a trigger slot
2. Choose the trigger type
3. Set the frequency range (for band triggers)
4. Adjust the threshold
5. Configure the OSC address and parameters

### Threshold Adjustment

- **Higher threshold**: Fewer, more significant triggers
- **Lower threshold**: More sensitive, may cause false triggers
- Use the visual feedback to find the right balance

---

## BPM Detection

sound2osc includes automatic beat detection.

### How It Works

1. Audio is analyzed for rhythmic patterns
2. Tempo is calculated from detected beats
3. BPM value is sent via OSC

### Manual Tap Tempo

If automatic detection isn't accurate:

1. Click the BPM display area
2. Tap in time with the music (minimum 4 taps)
3. The tempo will be calculated from your taps

### BPM Range

- Detection range: 60-200 BPM
- For faster/slower music, the detector may lock onto half/double time

---

## OSC Configuration

sound2osc sends OSC messages to control external software and hardware.

### Connection Setup

1. **Target IP**: The IP address of the receiving device (e.g., `192.168.1.100`)
2. **Target Port**: The OSC port to send to (e.g., `8000`)
3. **Local Port**: Port for receiving OSC (optional, e.g., `9000`)

### Default OSC Addresses

| Address | Parameters | Description |
|---------|------------|-------------|
| `/sound2osc/trigger/N` | float (0.0-1.0) | Trigger N intensity |
| `/sound2osc/level` | float (0.0-1.0) | Overall audio level |
| `/sound2osc/bpm` | float | Current BPM |
| `/sound2osc/beat` | - | Beat pulse (on each beat) |
| `/sound2osc/spectrum` | float[8] | Spectrum band levels |

See [OSC_REFERENCE.md](OSC_REFERENCE.md) for complete documentation.

---

## Presets

Presets save your trigger configurations for quick recall.

### Saving a Preset

1. Configure your triggers as desired
2. Click the preset menu
3. Select "Save As..."
4. Enter a name for your preset
5. Click Save

### Loading a Preset

1. Click the preset dropdown
2. Select a preset from the list
3. Settings are applied immediately

### Preset Location

Presets are stored in:
- **Linux**: `~/.config/sound2osc/presets/`
- **macOS**: `~/Library/Application Support/sound2osc/presets/`
- **Windows**: `%APPDATA%/sound2osc/presets/`

---

## Headless Mode

sound2osc can run without a GUI for server/embedded use.

### Basic Usage

```bash
# List available audio devices
./sound2osc-headless --list-devices

# Run with specific device
./sound2osc-headless --device "hw:0,0"

# Run with config file
./sound2osc-headless --config /path/to/config.json

# Verbose output
./sound2osc-headless --verbose
```

### Command-Line Options

| Option | Description |
|--------|-------------|
| `--help` | Show help message |
| `--version` | Show version information |
| `--list-devices` | List available audio devices |
| `--device <name>` | Specify audio input device |
| `--config <file>` | Load configuration from file |
| `--osc-host <ip>` | OSC target IP address |
| `--osc-port <port>` | OSC target port |
| `--verbose` | Enable verbose logging |
| `--quiet` | Minimal output |

### Running as a Service

On Linux with systemd:

```ini
# /etc/systemd/system/sound2osc.service
[Unit]
Description=sound2osc Audio Analysis Service
After=sound.target

[Service]
Type=simple
ExecStart=/usr/local/bin/sound2osc-headless --config /etc/sound2osc/config.json
Restart=on-failure
User=sound2osc

[Install]
WantedBy=multi-user.target
```

---

## Tips and Best Practices

### For Live Performance

1. Test your setup during soundcheck
2. Use conservative thresholds initially
3. Create multiple presets for different acts
4. Monitor the OSC status to ensure messages are being sent

### For Installations

1. Use headless mode for reliability
2. Configure auto-start with systemd or launchd
3. Use network monitoring to detect issues
4. Set up log rotation for long-running installations

### Reducing Latency

1. Use a dedicated audio interface (not built-in audio)
2. Reduce buffer sizes in your audio interface settings
3. Use wired network connections for OSC
4. Close unnecessary applications

---

## Next Steps

- [Configuration Reference](CONFIG_REFERENCE.md) - Detailed config options
- [OSC Reference](OSC_REFERENCE.md) - Complete OSC message documentation
