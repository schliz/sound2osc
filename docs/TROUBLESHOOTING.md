# Sound2OSC Troubleshooting Guide

This guide covers common issues and their solutions.

## Table of Contents

1. [Audio Issues](#audio-issues)
2. [OSC Issues](#osc-issues)
3. [Trigger Issues](#trigger-issues)
4. [BPM Detection Issues](#bpm-detection-issues)
5. [Build Issues](#build-issues)
6. [Performance Issues](#performance-issues)
7. [Platform-Specific Issues](#platform-specific-issues)

---

## Audio Issues

### No Audio Input Detected

**Symptoms:**
- Spectrum display shows no activity
- Level meter stays at zero
- "No audio device" error

**Solutions:**

1. **Check device connection**
   ```bash
   # Linux: List audio devices
   arecord -l
   
   # Or use PulseAudio
   pactl list sources
   ```

2. **Verify device permissions**
   ```bash
   # Linux: Add user to audio group
   sudo usermod -a -G audio $USER
   # Log out and back in for changes to take effect
   ```

3. **Select the correct device**
   - Open Sound2OSC
   - Click the audio input dropdown
   - Select your device from the list

4. **Check device in system settings**
   - Ensure the device isn't muted
   - Verify the device is set as default input

### Audio is Distorted/Clipping

**Symptoms:**
- Spectrum shows constant maximum values
- Triggers fire continuously
- Audible distortion

**Solutions:**

1. **Reduce input gain**
   - Lower the gain on your audio interface
   - Target peaks at 70-80% of the meter

2. **Check for DC offset**
   - Some cheap audio interfaces have DC offset issues
   - Use a different input or apply DC filtering

### Wrong Audio Device Selected

**Symptoms:**
- Audio works in other apps but not Sound2OSC
- Device shows "(unavailable)" in dropdown

**Solutions:**

1. **Refresh device list**
   - Restart Sound2OSC after connecting new devices

2. **Check device sampling rate**
   - Sound2OSC supports 44100 Hz and 48000 Hz
   - Ensure your device is set to a supported rate

---

## OSC Issues

### OSC Messages Not Received

**Symptoms:**
- Target application shows no incoming messages
- OSC status shows "0 messages sent"

**Solutions:**

1. **Verify network configuration**
   ```bash
   # Test network connectivity
   ping <target-ip>
   
   # Check if port is open
   nc -vzu <target-ip> <port>
   ```

2. **Check firewall settings**
   ```bash
   # Linux: Allow UDP traffic
   sudo ufw allow 8000/udp
   
   # Or for firewalld
   sudo firewall-cmd --add-port=8000/udp --permanent
   sudo firewall-cmd --reload
   ```

3. **Verify OSC settings in Sound2OSC**
   - Check target IP address is correct
   - Check target port matches receiving application
   - Ensure OSC client is enabled

4. **Test with OSC monitor**
   ```bash
   # Install oscdump (part of liblo-tools)
   sudo apt install liblo-tools
   
   # Monitor incoming OSC
   oscdump 8000
   ```

### OSC Messages Delayed

**Symptoms:**
- Triggers fire but response is slow
- Visible lag between audio and OSC events

**Solutions:**

1. **Reduce audio buffer size**
   - Smaller buffers = lower latency
   - Try 512 or 256 samples

2. **Use wired network**
   - WiFi adds latency and jitter
   - Ethernet provides more consistent timing

3. **Check network congestion**
   - Avoid sending too many messages
   - Reduce update rate if possible

### OSC Port Already in Use

**Symptoms:**
- "Address already in use" error
- OSC server won't start

**Solutions:**

1. **Find the process using the port**
   ```bash
   # Linux
   sudo lsof -i :8000
   
   # Or
   sudo netstat -tulpn | grep 8000
   ```

2. **Kill the conflicting process or use a different port**
   ```bash
   kill <pid>
   # Or change the port in Sound2OSC settings
   ```

---

## Trigger Issues

### Triggers Never Fire

**Symptoms:**
- Trigger indicators stay dark
- No OSC trigger messages sent
- Audio level appears normal

**Solutions:**

1. **Lower the threshold**
   - Default threshold may be too high for your audio
   - Start at 0.3 and adjust upward

2. **Check frequency range**
   - Ensure trigger frequency range matches your audio content
   - Bass trigger won't respond to high-frequency content

3. **Verify trigger is enabled**
   - Check the enable checkbox for each trigger
   - Ensure the trigger slot is not empty

### Triggers Fire Constantly

**Symptoms:**
- Trigger stays active continuously
- OSC messages flood the network
- May cause performance issues

**Solutions:**

1. **Raise the threshold**
   - Too low threshold causes false triggers
   - Increase until triggers respond to peaks only

2. **Check for DC offset or noise**
   - Constant low-level signal can trigger
   - Use a gate or noise reduction

3. **Check trigger hold time**
   - Ensure hold time is set appropriately
   - Longer hold time prevents rapid re-triggering

### Triggers Miss Events

**Symptoms:**
- Some audio peaks don't trigger
- Inconsistent trigger response
- Works sometimes but not always

**Solutions:**

1. **Lower the threshold slightly**
   - Find the balance between sensitivity and accuracy

2. **Check trigger attack time**
   - Fast transients may be missed with slow attack
   - Use faster attack for drums/percussion

3. **Increase audio input gain**
   - Weak signals may not reach threshold
   - Ensure consistent input level

---

## BPM Detection Issues

### Wrong BPM Detected

**Symptoms:**
- Displayed BPM is double or half the actual tempo
- BPM jumps around erratically
- Consistently wrong value

**Solutions:**

1. **Adjust BPM range**
   - Set min/max BPM to narrow the search range
   - For 120 BPM music, try 100-140 range

2. **Use tap tempo**
   - Tap the tempo manually to correct
   - System will lock to nearby tempo

3. **Improve audio quality**
   - BPM detection needs clear beat content
   - Avoid heavily compressed or distorted input

### BPM Not Detected

**Symptoms:**
- BPM shows 0 or no value
- Beat indicator never lights up
- Works with some music but not others

**Solutions:**

1. **Check audio input level**
   - Signal too weak for beat detection
   - Increase gain until peaks are clear

2. **Wait for detection**
   - BPM detection needs 2-4 seconds of audio
   - Be patient with new tracks

3. **Check music content**
   - Ambient/drone music may lack clear beats
   - BPM detection works best with rhythmic content

---

## Build Issues

### CMake Configuration Fails

**Symptoms:**
- "Could not find Qt6" error
- CMake errors about missing dependencies

**Solutions:**

See [Building on Linux](building/LINUX.md), [Building on macOS](building/MACOS.md), or [Building on Windows](building/WINDOWS.md) for platform-specific instructions.

Common fixes:

```bash
# Linux: Install Qt6 development packages
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-declarative-dev

# macOS: Install Qt via Homebrew
brew install qt@6
export Qt6_DIR=$(brew --prefix qt@6)/lib/cmake/Qt6

# Specify Qt path manually
cmake -B build -DQt6_DIR=/path/to/Qt/6.x/lib/cmake/Qt6
```

### Linker Errors

**Symptoms:**
- "undefined reference to..." errors
- Missing symbol errors at link time

**Solutions:**

1. **Clean build**
   ```bash
   rm -rf build
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```

2. **Check library versions**
   - Ensure all Qt6 packages are from the same version
   - Avoid mixing Qt5 and Qt6 components

### QML Module Not Found

**Symptoms:**
- "module 'QtQuick' is not installed" error
- Application crashes on startup

**Solutions:**

```bash
# Linux: Install QML modules
sudo apt install \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-dialogs \
    qml6-module-qtquick-window \
    qml6-module-qtmultimedia
```

---

## Performance Issues

### High CPU Usage

**Symptoms:**
- System becomes sluggish
- Fan runs constantly
- GUI becomes unresponsive

**Solutions:**

1. **Increase buffer size**
   - Larger buffers use less CPU
   - Try 2048 or 4096 samples

2. **Reduce spectrum resolution**
   - Fewer frequency bands = less processing

3. **Disable unused features**
   - Turn off BPM detection if not needed
   - Disable unused triggers

### Audio Dropouts

**Symptoms:**
- Clicking or popping in analysis
- Gaps in spectrum display
- Missed triggers during playback

**Solutions:**

1. **Close other applications**
   - Free up CPU and memory resources

2. **Use JACK (Linux) for lower latency**
   ```bash
   jackd -d alsa -r 44100 -p 256
   ```

3. **Check for thermal throttling**
   - Ensure adequate cooling
   - Check CPU temperature

---

## Platform-Specific Issues

### Linux

**PulseAudio/PipeWire Issues:**
```bash
# List available sources
pactl list sources short

# Set default source
pactl set-default-source <source-name>
```

**Permission Denied:**
```bash
# Add user to audio group
sudo usermod -a -G audio $USER
```

### macOS

**Microphone Permission:**
- Go to System Preferences > Security & Privacy > Privacy
- Enable microphone access for Sound2OSC

**Gatekeeper Warning:**
```bash
# Allow unsigned app (if building from source)
xattr -dr com.apple.quarantine /path/to/sound2osc.app
```

### Windows

**Audio Device Not Found:**
- Open Sound Settings
- Ensure device is enabled and set as default
- Check device isn't exclusively used by another app

**Firewall Blocking OSC:**
- Allow sound2osc.exe through Windows Firewall
- Allow both private and public networks if needed

---

## Getting Help

If you can't resolve your issue:

1. **Check the logs**
   ```bash
   # Run with verbose logging
   ./sound2osc-headless --verbose 2>&1 | tee debug.log
   ```

2. **Search existing issues**
   - Check the GitHub Issues for similar problems

3. **Report a bug**
   - Include: OS version, Qt version, steps to reproduce
   - Attach log output and configuration files

---

## See Also

- [User Guide](USER_GUIDE.md) - General usage
- [Configuration Reference](CONFIG_REFERENCE.md) - Settings options
- [Building on Linux](building/LINUX.md)
- [Building on Windows](building/WINDOWS.md)
- [Building on macOS](building/MACOS.md)
