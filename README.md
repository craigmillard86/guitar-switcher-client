# ESP32 Amp Channel Switcher

A sophisticated ESP32-based amplifier channel switcher with wireless control, MIDI support, and comprehensive logging capabilities. Supports multiple amp configurations through build-time configuration.

## Features

- **Multi-Channel Amp Switching**: Support for 2-4 amplifier channels via relays
- **Build-Time Configuration**: All pin assignments, device name, and channel count are set via `platformio.ini` and parsed at runtime
- **Runtime Pin Reporting**: Use the `pins` serial command to display all current pin assignments (amp switch, button, LED, MIDI)
- **Wireless Control**: ESP-NOW communication for wireless remote control
- **MIDI Support**: Program change messages for channel switching
- **OTA Updates**: Over-the-air firmware updates
- **Comprehensive Logging**: Multi-level logging with timestamps and NVS persistence
- **Serial Commands**: Extensive command interface for debugging and control
- **Performance Monitoring**: Real-time performance and memory tracking
- **Auto-Pairing**: Automatic device pairing system
- **Button Support**: Physical buttons for direct channel switching
- **LED Feedback Patterns**: Status LED provides feedback for events (data, command, MIDI, OTA, error, pairing)

## Build-Time Configuration System

All hardware pin assignments, feature flags, and channel counts are now set in `config.h` or via build flags in `platformio.ini`. The `CLIENT_TYPE` macro determines the device's role and features at build time. At runtime, the firmware parses these macros and reports the actual configuration via the `config` and `pins` serial commands.

### How CLIENT_TYPE Works
- Each build environment in `platformio.ini` sets a `CLIENT_TYPE` (e.g., `AMP_SWITCHER`)
- Other flags (e.g., `MAX_AMPSWITCHS`, `AMP_SWITCH_PINS`, `DEVICE_NAME`) are set per environment
- The firmware adapts its features, pins, and commands based on these flags

#### Example: Adding a New Client Type
To add a new device type or configuration:
1. Add a new environment in `platformio.ini`:
```ini
[env:client-custom-amp]
extends = env:esp32-c3-devkitc-02
build_flags = 
    -D CLIENT_TYPE=AMP_SWITCHER
    -D MAX_AMPSWITCHS=3
    -D AMP_SWITCH_PINS="4,5,6"
    -D AMP_BUTTON_PINS="8,9,10"
    -D DEVICE_NAME="CUSTOM_AMP"
```
2. Build with the new configuration:
```bash
platformio run -e client-custom-amp
```

### Viewing the Current Configuration
- Use the `config` serial command to display the current build configuration, including client type, pins, and device name.
- Use the `pins` serial command to display all runtime pin assignments (amp switch, button, LED, MIDI).

## Hardware Setup

### Pin Configuration
- **Amp Switch Pins**: Set via `AMP_SWITCH_PINS` in `platformio.ini` (e.g., GPIO 2, 3, 4, 5)
- **Amp Button Pins**: Set via `AMP_BUTTON_PINS` in `platformio.ini` (e.g., GPIO 8, 9, 10, 20)
- **Status/Pairing LED**: Set via `PAIRING_LED_PIN` (default: GPIO 1, reserved for LED only)
- **MIDI RX**: Set via `MIDI_RX_PIN` (default: GPIO 6)
- **MIDI TX**: Set via `MIDI_TX_PIN` (default: GPIO 7)

> **Note:** GPIO 1 is reserved for the status/pairing LED. Do **not** use GPIO 1 for relays or switches to avoid conflicts.

**Button 1 (amp channel 1 button) now has multiple functions:**
- **Short press (<5s):** Switch to channel 1
- **Long press (>5s):** Enter pairing mode (after setup window)
- **Long press during setup (hold for 5s within first 10s after boot):** Enter OTA mode

> **Note:** There is no longer a dedicated OTA button. OTA mode can only be triggered by serial command (`ota`) or by holding Button 1 for 5 seconds during the setup window after boot.

### Supported Configurations

| Configuration | Channels | Switch Pins | Button Pins | Device Name |
|---------------|----------|-------------|-------------|-------------|
| `client-2ch-amp` | 2 | 4, 5 | 8, 9 | 2CH_AMP |
| `client-4ch-amp` | 4 | 2, 3, 4, 5 | 8, 9, 10, 20 | 4CH_AMP |
| `client-amp-switcher` | 4 | 2, 3, 4, 5 | 8, 9, 10, 20 | AMP_SWITCHER_1 |

> **Note:** GPIO 20 is used for the 4th amp button on the ESP32-C3 Super Mini. GPIO 11 is not available on this board.

## Serial Commands

### System Commands
| Command | Description |
|---------|-------------|
| `help` | Show complete help menu |
| `status` | Show complete system status |
| `config` | Show client configuration (all runtime config, device name, pins) |
| `pins` | Show all runtime pin assignments (amp switch, button, LED, MIDI) |
| `memory` | Show memory usage |
| `network` | Show network status |
| `amp` | Show amp channel status |
| `pairing` | Show pairing status |
| `uptime` | Show system uptime |
| `version` | Show firmware version |
| `midi` | Show MIDI configuration |
| `buttons` | Toggle button checking on/off |

### Control Commands
| Command | Description |
|---------|-------------|
| `restart` | Reboot the device |
| `ota` | Enter OTA update mode (only available during setup window, or via serial) |
| `pair` | Clear pairing and re-pair |
| `setlogN` | Set log level (N=0-4) |
| `clearlog` | Clear saved log level |
| `clearall` | Clear all NVS data |

### Button 1 Functions
| Action | When | Result |
|--------|------|--------|
| Short press (<5s) | Any time | Switch to channel 1 |
| Long press (>5s) | After setup window | Enter pairing mode |
| Long press (>5s) | During setup window (first 10s after boot) | Enter OTA mode |

### Test Commands
| Command | Description |
|---------|-------------|
| `testled` | Test status LED |
| `testpairing` | Test pairing LED |

### Debug Commands
| Command | Description |
|---------|-------------|
| `debug` | Show complete debug info |
| `debugperf` | Show performance metrics |
| `debugmemory` | Show memory analysis |
| `debugwifi` | Show WiFi stats |
| `debugespnow` | Show ESP-NOW stats |
| `debugtask` | Show task stats |
| `debughelp` | Show debug commands |

### Amp Channel Commands
| Command | Description |
|---------|-------------|
| `1-4` | Switch to amp channel 1-4 |
| `b1-b4` | Simulate button press 1-4 |
| `off` | Turn all channels off |

### Example Output for `pins` Command
```
[INFO] === PIN ASSIGNMENTS ===
[INFO] Amp Switch Pins: 2,3,4,5
[INFO] Amp Button Pins: 8,9,10,20
[INFO] Status/Pairing LED Pin: 1
[INFO] MIDI RX Pin: 6
[INFO] MIDI TX Pin: 7
[INFO] ======================
```

### Example Output for `config` Command
```
[INFO] === CLIENT CONFIGURATION ===
[INFO] Client Type: AMP_SWITCHER
[INFO] Device Name: AMP_SWITCHER_1
[INFO] Amp Switching: Enabled
[INFO] Max Amp Switches: 4
[INFO] Amp Switch Pins: 2,3,4,5
[INFO] Amp Button Pins: 8,9,10,20
[INFO] ===========================
```

## LED Feedback Patterns
- **Single Flash:** ESP-NOW data received
- **Double Flash:** Serial command or ESP-NOW command received
- **Triple Flash:** MIDI message received
- **Fast Blink:** OTA update in progress
- **Solid On:** Error state
- **Fade/Breath:** Pairing mode

## Troubleshooting

- If the `pins` or `config` command does not match your expected configuration, check your `platformio.ini` build flags and rebuild the firmware.
- All configuration is dynamic and reported at runtime.

## Building and Flashing

### Prerequisites
1. Install PlatformIO
2. Open the project in PlatformIO
3. Configure your board in `platformio.ini`

### Build Configurations

The project supports multiple build configurations for different amp setups:

```bash
# For 2-channel amp
platformio run -e client-2ch-amp

# For 4-channel amp
platformio run -e client-4ch-amp

# For original amp switcher
platformio run -e client-amp-switcher

# Build and upload
platformio run -e client-2ch-amp --target upload
```

### Configuration Options

Each build environment defines:
- **Channel Count**: Number of amp channels (2 or 4)
- **Pin Assignments**: GPIO pins for switches and buttons
- **Device Name**: Unique identifier for the device

## Configuration

### Build-Time Configuration

Configuration is handled through PlatformIO build flags in `platformio.ini`:

```ini
[env:client-2ch-amp]
build_flags = 
    -D CLIENT_TYPE=AMP_SWITCHER
    -D MAX_AMPSWITCHS=2
    -D AMP_SWITCH_PINS="4,5"
    -D AMP_BUTTON_PINS="8,9"
    -D DEVICE_NAME="2CH_AMP"
```

### Adding New Configurations

To add a new amp configuration:

1. Add a new environment in `platformio.ini`:
```ini
[env:client-custom-amp]
extends = env:esp32-c3-devkitc-02
build_flags = 
    -D CLIENT_TYPE=AMP_SWITCHER
    -D MAX_AMPSWITCHS=3
    -D AMP_SWITCH_PINS="4,5,6"
    -D AMP_BUTTON_PINS="8,9,10"
    -D DEVICE_NAME="CUSTOM_AMP"
```

2. Build with the new configuration:
```bash
platformio run -e client-custom-amp
```

## Troubleshooting

- If the `pins` or `config` command does not match your expected configuration, check your `platformio.ini` build flags and rebuild the firmware.
- All configuration is dynamic and reported at runtime. 