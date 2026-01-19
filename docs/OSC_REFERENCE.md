# Sound2OSC OSC Protocol Reference

This document describes all OSC messages sent and received by Sound2OSC.

## Overview

Sound2OSC uses OSC (Open Sound Control) to communicate with external software and hardware. Messages are sent over UDP.

### Default Ports

| Direction | Port | Description |
|-----------|------|-------------|
| Outgoing | 8000 | Messages sent to external devices |
| Incoming | 9000 | Messages received from external devices |

---

## Outgoing Messages

### Trigger Messages

#### /sound2osc/trigger/[n]

Sent when a trigger fires.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| intensity | float | 0.0-1.0 | Trigger intensity/velocity |

**Example:**
```
/sound2osc/trigger/1 0.85
```

#### /sound2osc/trigger/[n]/state

Sent when a trigger state changes (on/off).

| Parameter | Type | Values | Description |
|-----------|------|--------|-------------|
| state | int | 0, 1 | 0 = off, 1 = on |

**Example:**
```
/sound2osc/trigger/1/state 1
```

---

### Level Messages

#### /sound2osc/level

Overall audio input level.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| level | float | 0.0-1.0 | Current audio level |

**Update rate:** ~44 Hz (every audio frame)

**Example:**
```
/sound2osc/level 0.72
```

#### /sound2osc/peak

Peak audio level with decay.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| peak | float | 0.0-1.0 | Peak level |

**Example:**
```
/sound2osc/peak 0.95
```

---

### Spectrum Messages

#### /sound2osc/spectrum

Full spectrum data as array.

| Parameter | Type | Count | Description |
|-----------|------|-------|-------------|
| bands | float[] | 8 | Frequency band levels (0.0-1.0) |

**Band mapping:**

| Index | Frequency Range |
|-------|-----------------|
| 0 | 20-60 Hz (Sub bass) |
| 1 | 60-150 Hz (Bass) |
| 2 | 150-400 Hz (Low mids) |
| 3 | 400-1000 Hz (Mids) |
| 4 | 1000-2500 Hz (Upper mids) |
| 5 | 2500-6000 Hz (Presence) |
| 6 | 6000-12000 Hz (Brilliance) |
| 7 | 12000-20000 Hz (Air) |

**Example:**
```
/sound2osc/spectrum 0.9 0.7 0.5 0.4 0.3 0.2 0.15 0.1
```

#### /sound2osc/spectrum/[n]

Individual spectrum band.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| level | float | 0.0-1.0 | Band level |

**Example:**
```
/sound2osc/spectrum/0 0.9
```

---

### BPM Messages

#### /sound2osc/bpm

Current detected BPM.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| bpm | float | 0-300 | Beats per minute |

**Update rate:** When BPM changes

**Example:**
```
/sound2osc/bpm 128.5
```

#### /sound2osc/beat

Beat pulse - sent on each detected beat.

| Parameter | Type | Description |
|-----------|------|-------------|
| (none) | - | Message sent on beat |

**Example:**
```
/sound2osc/beat
```

#### /sound2osc/beat/phase

Current position within beat cycle.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| phase | float | 0.0-1.0 | Position in beat (0 = on beat) |

**Example:**
```
/sound2osc/beat/phase 0.25
```

---

### Status Messages

#### /sound2osc/status

Application status updates.

| Parameter | Type | Description |
|-----------|------|-------------|
| status | string | Status message |

**Example:**
```
/sound2osc/status "running"
```

#### /sound2osc/error

Error notifications.

| Parameter | Type | Description |
|-----------|------|-------------|
| code | int | Error code |
| message | string | Error description |

**Example:**
```
/sound2osc/error 100 "Audio device disconnected"
```

---

## Incoming Messages

Sound2OSC can receive OSC messages to control its operation.

### Control Messages

#### /sound2osc/control/enable

Enable/disable triggers.

| Parameter | Type | Description |
|-----------|------|-------------|
| trigger_id | int | Trigger number (1-16) |
| enabled | int | 0 = disable, 1 = enable |

**Example:**
```
/sound2osc/control/enable 1 1
```

#### /sound2osc/control/threshold

Set trigger threshold.

| Parameter | Type | Description |
|-----------|------|-------------|
| trigger_id | int | Trigger number (1-16) |
| threshold | float | New threshold (0.0-1.0) |

**Example:**
```
/sound2osc/control/threshold 1 0.75
```

#### /sound2osc/control/preset

Load a preset.

| Parameter | Type | Description |
|-----------|------|-------------|
| name | string | Preset name |

**Example:**
```
/sound2osc/control/preset "Rock Band"
```

---

## Message Bundling

For efficiency, Sound2OSC bundles related messages together when possible. OSC bundles are timestamped using NTP format.

### Bundle Example

```
#bundle
  timetag: 1234567890.123456789
  /sound2osc/level 0.72
  /sound2osc/spectrum 0.9 0.7 0.5 0.4 0.3 0.2 0.15 0.1
  /sound2osc/trigger/1 0.85
```

---

## Integration Examples

### TouchOSC

Configure a control surface to receive trigger values:

1. Set OSC Host to Sound2OSC IP
2. Set OSC Port to 8000
3. Map faders/buttons to `/sound2osc/trigger/[n]`

### QLab

Receive triggers for cue firing:

1. Add OSC Cue
2. Set network patch to Sound2OSC port (9000)
3. Trigger on `/sound2osc/beat` or `/sound2osc/trigger/[n]`

### Max/MSP

Receive spectrum data:

```max
[udpreceive 8000]
|
[oscparse]
|
[route /sound2osc/spectrum]
|
[unpack f f f f f f f f]
```

### Pure Data

Receive BPM:

```pd
[netreceive -u -b 8000]
|
[oscparse]
|
[routeOSC /sound2osc/bpm]
|
[float]
```

### Python (python-osc)

```python
from pythonosc import dispatcher, osc_server

def handle_trigger(address, *args):
    trigger_num = address.split('/')[-1]
    intensity = args[0]
    print(f"Trigger {trigger_num}: {intensity}")

dispatcher = dispatcher.Dispatcher()
dispatcher.map("/sound2osc/trigger/*", handle_trigger)

server = osc_server.ThreadingOSCUDPServer(("0.0.0.0", 8000), dispatcher)
server.serve_forever()
```

---

## Troubleshooting

### No Messages Received

1. Check that OSC client is enabled in Sound2OSC
2. Verify target IP and port are correct
3. Check firewall settings on both machines
4. Use a network analyzer (Wireshark) to verify packets are sent

### Messages Delayed

1. Reduce audio buffer size in Sound2OSC
2. Use wired network connection instead of WiFi
3. Ensure receiving application can process messages fast enough

### Wrong Data Format

Verify your receiving application expects the correct OSC type tags:
- `f` = 32-bit float
- `i` = 32-bit integer
- `s` = string
- `b` = blob (binary data)

---

## See Also

- [User Guide](USER_GUIDE.md) - General usage instructions
- [Configuration Reference](CONFIG_REFERENCE.md) - Configuration options
- [OSC Specification](http://opensoundcontrol.org/spec-1_0) - Official OSC 1.0 spec
