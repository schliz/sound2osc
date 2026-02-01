# sound2osc Configuration Reference

This document describes all configuration options for sound2osc.

## Configuration Files

### Location

Configuration files are stored in platform-specific locations:

| Platform | Path |
|----------|------|
| Linux | `~/.config/sound2osc/` |
| macOS | `~/Library/Application Support/sound2osc/` |
| Windows | `%APPDATA%/sound2osc/` |

### File Types

| File | Purpose |
|------|---------|
| `settings.json` | Application settings |
| `presets/*.json` | Trigger presets |

---

## Settings File (settings.json)

### Complete Example

```json
{
  "version": 1,
  "audio": {
    "device": "default",
    "sampleRate": 44100,
    "bufferSize": 1024
  },
  "osc": {
    "client": {
      "enabled": true,
      "host": "127.0.0.1",
      "port": 8000
    },
    "server": {
      "enabled": false,
      "port": 9000
    }
  },
  "ui": {
    "windowGeometry": "...",
    "windowState": "...",
    "theme": "dark"
  },
  "logging": {
    "level": "info",
    "file": "",
    "console": true
  }
}
```

---

## Audio Settings

### audio.device

The audio input device to use.

- **Type**: string
- **Default**: `"default"`
- **Examples**: `"default"`, `"hw:0,0"`, `"Built-in Microphone"`

### audio.sampleRate

Sample rate for audio capture.

- **Type**: integer
- **Default**: `44100`
- **Valid values**: `44100`, `48000`

### audio.bufferSize

Audio buffer size in samples.

- **Type**: integer
- **Default**: `1024`
- **Valid values**: `256`, `512`, `1024`, `2048`, `4096`
- **Note**: Smaller values = lower latency but higher CPU usage

---

## OSC Settings

### osc.client.enabled

Enable sending OSC messages.

- **Type**: boolean
- **Default**: `true`

### osc.client.host

Target IP address for OSC messages.

- **Type**: string
- **Default**: `"127.0.0.1"`
- **Examples**: `"192.168.1.100"`, `"10.0.0.50"`

### osc.client.port

Target UDP port for OSC messages.

- **Type**: integer
- **Default**: `8000`
- **Range**: 1-65535

### osc.server.enabled

Enable receiving OSC messages.

- **Type**: boolean
- **Default**: `false`

### osc.server.port

Local UDP port for receiving OSC.

- **Type**: integer
- **Default**: `9000`
- **Range**: 1-65535

---

## UI Settings

### ui.windowGeometry

Saved window position and size (base64 encoded).

- **Type**: string
- **Note**: Managed automatically by the application

### ui.windowState

Saved window state (maximized, etc.).

- **Type**: string
- **Note**: Managed automatically by the application

### ui.theme

Color theme for the application.

- **Type**: string
- **Default**: `"dark"`
- **Valid values**: `"dark"`, `"light"`

---

## Logging Settings

### logging.level

Minimum log level to output.

- **Type**: string
- **Default**: `"info"`
- **Valid values**: `"debug"`, `"info"`, `"warning"`, `"error"`, `"critical"`

### logging.file

Path to log file.

- **Type**: string
- **Default**: `""` (no file logging)
- **Example**: `"/var/log/sound2osc.log"`

### logging.console

Enable console logging.

- **Type**: boolean
- **Default**: `true`

---

## Preset Files

Preset files store trigger configurations.

### Preset Example

```json
{
  "version": 1,
  "name": "Rock Band",
  "triggers": [
    {
      "id": 1,
      "enabled": true,
      "type": "band",
      "freqLow": 20,
      "freqHigh": 150,
      "threshold": 0.6,
      "oscAddress": "/sound2osc/trigger/1"
    },
    {
      "id": 2,
      "enabled": true,
      "type": "band",
      "freqLow": 2000,
      "freqHigh": 8000,
      "threshold": 0.5,
      "oscAddress": "/sound2osc/trigger/2"
    }
  ],
  "bpm": {
    "enabled": true,
    "minBpm": 80,
    "maxBpm": 180
  }
}
```

---

## Trigger Configuration

### trigger.id

Unique identifier for the trigger.

- **Type**: integer
- **Range**: 1-16

### trigger.enabled

Whether this trigger is active.

- **Type**: boolean
- **Default**: `true`

### trigger.type

Type of trigger detection.

- **Type**: string
- **Valid values**: `"level"`, `"band"`, `"onset"`, `"bpm"`

### trigger.freqLow

Lower frequency bound for band triggers.

- **Type**: integer
- **Default**: `20`
- **Range**: 20-20000 Hz

### trigger.freqHigh

Upper frequency bound for band triggers.

- **Type**: integer
- **Default**: `20000`
- **Range**: 20-20000 Hz

### trigger.threshold

Trigger threshold level.

- **Type**: float
- **Default**: `0.5`
- **Range**: 0.0-1.0

### trigger.oscAddress

OSC address for this trigger's messages.

- **Type**: string
- **Default**: `"/sound2osc/trigger/N"` (where N is the trigger ID)

---

## BPM Configuration

### bpm.enabled

Enable BPM detection.

- **Type**: boolean
- **Default**: `true`

### bpm.minBpm

Minimum BPM to detect.

- **Type**: integer
- **Default**: `60`
- **Range**: 20-200

### bpm.maxBpm

Maximum BPM to detect.

- **Type**: integer
- **Default**: `200`
- **Range**: 60-300

---

## Command-Line Overrides

Many settings can be overridden via command-line arguments (headless mode):

| Setting | Command-Line |
|---------|--------------|
| audio.device | `--device <name>` |
| osc.client.host | `--osc-host <ip>` |
| osc.client.port | `--osc-port <port>` |
| logging.level | `--verbose` / `--quiet` |

---

## Environment Variables

| Variable | Description |
|----------|-------------|
| `SOUND2OSC_CONFIG_DIR` | Override config directory location |
| `SOUND2OSC_LOG_LEVEL` | Set log level (debug/info/warning/error) |

---

## Migration from Legacy Settings

If you have settings from an older version (using INI format), sound2osc will automatically migrate them on first run. A backup of your old settings is created at `settings.ini.backup`.

---

## See Also

- [User Guide](USER_GUIDE.md) - General usage instructions
- [OSC Reference](OSC_REFERENCE.md) - OSC message documentation
- [Troubleshooting](TROUBLESHOOTING.md) - Common issues
