# ESP32 Guitar Amp Channel Switcher - Command Reference

> **📖 For complete usage guide, see the main [README.md](README.md)**

This document provides a comprehensive reference for all serial commands available in the ESP32 Guitar Amp Channel Switcher.

## Quick Start Commands

Connect via USB serial monitor (115200 baud) and try these essential commands:

| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `status` | Show current channel and MIDI settings |
| `midimap` | Show Program Change to channel mappings |
| `pins` | Show current pin assignments |
| `config` | Show build configuration details |

## Channel Control Commands

| Command | Description |
|---------|-------------|
| `1` | Switch to channel 1 |
| `2` | Switch to channel 2 |
| `3` | Switch to channel 3 |
| `4` | Switch to channel 4 |
| `off` | Turn all channels off |
| `b1` | Simulate button 1 press |
| `b2` | Simulate button 2 press |
| `b3` | Simulate button 3 press |
| `b4` | Simulate button 4 press |

## MIDI Commands

| Command | Description |
|---------|-------------|
| `midimap` | Show Program Change to channel mappings |
| `midi` | Show MIDI configuration and current channel |

### MIDI Learn Process
**Single Channel Mode:**
1. Hold Button 1 for 10+ seconds → Release (LED blinks fast)
2. Send Program Change from your controller
3. Done - PC number now controls the channel

**Multi-Channel Mode:**
1. Hold Button 1 for 10+ seconds → Release (LED blinks fast)  
2. Press channel button you want to program (1-4)
3. Send Program Change from your controller
4. Done - PC number now switches to that channel

> **Note:** 30-second timeout, 2-second cooldown after learning

## System Information Commands

| Command | Description |
|---------|-------------|
| `status` | Complete system status |
| `config` | Build configuration details |
| `pins` | Runtime pin assignments |
| `version` | Firmware and storage versions |
| `uptime` | System uptime |
| `memory` | Memory usage statistics |
| `network` | Network/WiFi status |
| `amp` | Amp channel status |
| `pairing` | Pairing status |

## Logging Commands

| Command | Description |
|---------|-------------|
| `setlog0` | Turn off all logging |
| `setlog1` | Show errors only |
| `setlog2` | Show warnings and errors |
| `setlog3` | Show info, warnings, and errors (default) |
| `setlog4` | Show all messages including debug |
| `loglevel` | Show current log level |
| `clearlog` | Reset log level to default |

## Debug Commands

| Command | Description |
|---------|-------------|
| `debug` | Complete system debug info |
| `debugperf` | Performance metrics |
| `debugmemory` | Memory analysis |
| `debugwifi` | WiFi statistics |
| `debugespnow` | ESP-NOW wireless statistics |
| `debugtask` | Task statistics |
| `debughelp` | Show debug command help |

## Maintenance Commands

| Command | Description |
|---------|-------------|
| `restart` | Reboot device |
| `ota` | Enter firmware update mode |
| `pair` | Clear pairing and re-pair |
| `buttons` | Toggle button checking on/off |
| `clearall` | Reset all NVS settings to defaults |
| `clearlog` | Reset log level only |

## Test Commands

| Command | Description |
|---------|-------------|
| `testled` | Test status LED patterns |
| `testpairing` | Test pairing LED |

---

## Current Hardware Configuration

Based on current `platformio.ini` build settings:

### Pin Assignments
- **Relay Control**: GPIO 2 (amp channel switching)
- **Button Input**: GPIO 1 (manual channel control)
- **Status LED**: GPIO 8 (visual feedback)
- **MIDI Input**: GPIO 6 (from MIDI controller)
- **MIDI Output**: GPIO 7 (MIDI thru)

> **Note:** Current configuration is single-channel mode (MAX_AMPSWITCHS=1)

### Example Command Outputs

**`pins` Command:**
```
[INFO] === PIN ASSIGNMENTS ===
[INFO] Amp Switch Pins: 2
[INFO] Amp Button Pins: 1
[INFO] Status/Pairing LED Pin: 8
[INFO] MIDI RX Pin: 6
[INFO] MIDI TX Pin: 7
[INFO] ======================
```

**`midimap` Command:**
```
[INFO] === MIDI PROGRAM CHANGE MAP ===
[INFO] Channel 1: PC#0
[INFO] Channel 2: PC#1
[INFO] Channel 3: PC#2
[INFO] Channel 4: PC#3
[INFO] ==============================
```

**`config` Command:**
```
[INFO] === CLIENT CONFIGURATION ===
[INFO] Client Type: AMP_SWITCHER
[INFO] Device Name: AMP_SWITCHER_1
[INFO] Amp Switching: Enabled
[INFO] Max Amp Switches: 1
[INFO] Amp Switch Pins: 2
[INFO] Amp Button Pins: 1
[INFO] ===========================
```

---

## Button Functions Summary

### Button 1 Multi-Function:
| Hold Time | Action | Result |
|-----------|--------|--------|
| < 5s | Short press | Switch to channel 1 |
| 10s+ | Release after hold | Enter MIDI Learn mode |
| 15s+ | Release after hold | Enter MIDI channel select mode (1-16) |
| 30s+ | Release after hold | Enter pairing mode |
| 5s (boot) | During first 10s after power-on | Enter OTA update mode |

### LED Feedback During Long Press:
- LED flashes at 5s, 10s, 15s, 20s, 25s intervals for timing feedback

---

## LED Status Indicators

| Pattern | Meaning |
|---------|---------|
| **Single Flash** | Button timing feedback, MIDI learned successfully |
| **Triple Flash** | MIDI message received, switching channels |
| **Fast Blink** | MIDI Learn mode active (waiting for controller) |
| **Fade/Breath** | Channel select mode or pairing mode active |
| **OFF** | Normal operation |

---

## OTA Firmware Updates

### Method 1: Serial Command
1. Type `ota` in serial monitor
2. Connect to WiFi `ESP32_OTA` (password: `12345678`)
3. Go to `http://192.168.4.1/update`
4. Upload firmware file

### Method 2: Button During Boot
1. Power on device
2. Within first 10 seconds: Hold Button 1 for 5+ seconds
3. Follow steps 2-4 above

> **Note:** OTA mode lasts 5 minutes per session

---

## Common Usage Examples

```bash
# Check system status and current settings
status

# View current MIDI mappings
midimap

# Switch to channel 3
3

# Enable detailed logging
setlog4

# Check memory usage
debugmemory

# Reset all settings to defaults
clearall

# Reboot device
restart

# Enter firmware update mode
ota
```

---

## Troubleshooting Quick Reference

| Problem | Command to Try | Description |
|---------|---------------|-------------|
| No response | `restart` | Reboot device |
| Can't see logs | `setlog4` | Enable all logging |
| MIDI not working | `midi` | Check MIDI configuration |
| Buttons not working | `buttons` | Toggle button checking |
| Memory issues | `debugmemory` | Check memory usage |
| Pairing problems | `pair` | Reset pairing |
| Wrong configuration | `pins` | Verify pin assignments |
| Settings corrupted | `clearall` | Reset all settings |

---

## Advanced Technical Information

For detailed technical specifications, build configuration, and architecture details, see the main [README.md](README.md#build-configuration--technical-details).

### Storage System
- All settings stored in NVS (non-volatile storage)
- Version control prevents corruption during upgrades
- Settings persist across power cycles
- Manual reset available with `clearall`

### Wireless Communication  
- ESP-NOW protocol for low-latency control
- Automatic pairing system
- Use `debugespnow` to monitor wireless status

### Performance Monitoring
- Real-time memory tracking with `debugmemory`
- Performance metrics with `debugperf`
- System uptime with `uptime`
- `