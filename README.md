# ESP32 Amp Channel Switcher

A sophisticated ESP32-based amplifier channel switcher with wireless control, MIDI support, and comprehensive logging capabilities. Supports multiple amp configurations through build-time configuration.

## Features

- **Multi-Channel Amp Switching**: Support for 2-4 amplifier channels via relays
- **Build-Time Configuration**: All pin assignments, device name, and channel count are set via `platformio.ini` and parsed at runtime
- **Runtime Pin Reporting**: Use the `pins` serial command to display all current pin assignments (amp switch, button, LED, MIDI)
- **Wireless Control**: ESP-NOW communication for wireless remote control
- **MIDI Support**: Program change messages for channel switching
- **OTA Updates (AP + ElegantOTA)**: Over-the-air firmware updates via built-in Access Point and ElegantOTA web interface
- **Comprehensive Logging**: Multi-level logging with timestamps and NVS persistence
- **Serial Commands**: Extensive command interface for debugging and control
- **Performance Monitoring**: Real-time performance and memory tracking
- **Auto-Pairing**: Automatic device pairing system
- **Button Support**: Physical buttons for direct channel switching
- **LED Feedback Patterns**: Status LED provides feedback for events (data, command, MIDI, OTA, error, pairing)

## OTA Updates (Access Point + ElegantOTA)

**New in this version:**
- OTA updates are performed by starting the ESP32 as a WiFi Access Point (`ESP32_OTA`, default IP: `192.168.4.1`).
- ElegantOTA provides a web interface for firmware upload at `http://192.168.4.1/update`.
- No WiFiManager or captive portal is used—OTA is always available, even with no WiFi configured.

### How to Use OTA Update

1. **Trigger OTA mode** (via serial command `ota`, button, or at boot).
2. The ESP32 starts an AP named `ESP32_OTA` (password: `12345678`).
3. Connect your phone or computer to this AP.
4. Open a browser and go to [http://192.168.4.1/update](http://192.168.4.1/update).
5. Upload your new firmware.
6. The ESP32 will reboot and run the new firmware.

**OTA mode is available for 5 minutes per session.**  
The status LED will indicate OTA mode (fast blink or custom pattern).

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
- **Amp Switch Pins**: Set via `AMP_SWITCH_PINS` in `platformio.ini` (e.g., GPIO 2, 9, 10, 20)
- **Amp Button Pins**: Set via `AMP_BUTTON_PINS` in `platformio.ini` (e.g., GPIO 1, 3, 4, 5)
- **Status/Pairing LED**: Set via `PAIRING_LED_PIN` (default: GPIO 8, reserved for LED only)
- **MIDI RX**: Set via `MIDI_RX_PIN` (default: GPIO 6)
- **MIDI TX**: Set via `MIDI_TX_PIN` (default: GPIO 7)

> **Note:** GPIO 8 is reserved for the status/pairing LED. Do **not** use GPIO 8 for relays or switches to avoid conflicts.

## Button 1 Functions (Unified Logic)

| Action                        | When                                 | Result                                                                                 |
|-------------------------------|--------------------------------------|----------------------------------------------------------------------------------------|
| **Short press (<5s)**         | Any time (single-button mode)        | Toggle relay ON/OFF                                                                    |
| **Short press (<5s)**         | Any time (multi-button mode)         | Switch to channel 1                                                                    |
| **15s long press**            | Any time                             | Enter channel select mode (both modes)                                                 |
| **Press in channel select**   | While in channel select mode         | Increment MIDI channel (cycles 1-16), LED flashes selected channel number              |
| **30s long press**            | Any time                             | Enter pairing mode                                                                     |
| **Double long press (5s, release, 5s)** | Any time (single-button mode) | Enter MIDI Learn mode                                                                  |
| **Long press (5s during setup window)** | First 10s after boot          | Enter OTA mode                                                                         |
| **Long press milestones**     | Any time                             | LED flashes at 5s, 10s, 15s, 20s, 25s to provide timing feedback                       |

### Summary of Key Changes
- Channel select mode is now entered with a 15s hold, and works in both single and multi-button modes.
- Relay toggle on short press is only for single-button mode; in multi-button mode, short press switches to channel 1.
- Pairing mode is now a 30s hold.
- MIDI Learn is a double long-press (single-button mode only).
- OTA mode is a 5s long press during the setup window (first 10s after boot).
- LED feedback is unified and milestone-based.

### Supported Configurations

| Configuration | Channels | Switch Pins | Button Pins | Device Name |
|---------------|----------|-------------|-------------|-------------|
| `client-2ch-amp` | 2 | 2, 9 | 1, 3 | 2CH_AMP |
| `client-4ch-amp` | 4 | 2, 9, 10, 20 | 1, 3, 4, 5 | 4CH_AMP |

## Serial Commands

### System Commands
| Command | Description |
|---------|-------------|
| `help` | Show complete help menu |
| `status` | Show complete system status |
| `config` | Show client configuration (all runtime config, device name, pins) |
| `pins` | Show all runtime pin assignments (amp switch, button, LED, MIDI) |
| `midimap` | Show MIDI Program Change to channel mapping |
| `memory` | Show memory usage |
| `network` | Show network status |
| `amp` | Show amp channel status |
| `pairing` | Show pairing status |
| `uptime` | Show system uptime |
| `version` | Show firmware version |
| `midi` | Show MIDI configuration |
| `buttons` | Toggle button checking on/off |

### MIDI Learn Feature
- **To enter MIDI Learn mode:** Hold Button 1 and Button 2 together for >2 seconds. The device will indicate MIDI Learn mode is armed (LED/serial).
- **To select a channel:** Release both, then short-press the desired channel button (b1–b4). The device will wait for a MIDI Program Change message.
- **To assign:** Send a MIDI Program Change from your controller. The received PC number will be mapped to the selected channel and saved to NVS.
- **To view mapping:** Use the `midimap` serial command.
- **Mapping is persistent** and can be changed at any time using the above process.

#### Example Output for `midimap` Command
```
[INFO] === MIDI PROGRAM CHANGE MAP ===
[INFO] Channel 1: PC#0
[INFO] Channel 2: PC#1
[INFO] Channel 3: PC#2
[INFO] Channel 4: PC#3
[INFO] ==============================
```

### Control Commands
| Command | Description |
|---------|-------------|
| `restart` | Reboot the device |
| `ota` | Enter OTA update mode (serial command, or hold Button 1 for 5s during boot) |
| `pair` | Clear pairing and re-pair |
| `setlogN` | Set log level (N=0-4) |
| `clearlog` | Clear saved log level |
| `clearall` | Clear all NVS data |

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

## NVS Storage Versioning

All persistent settings (pairing info, log level, MIDI mapping) are stored in NVS (non-volatile storage) with a version number (`STORAGE_VERSION`).
- On firmware upgrade, if the stored version does not match the current firmware's `STORAGE_VERSION`, the affected settings are reset to safe defaults and the new version is saved.
- This ensures safe upgrades and prevents configuration corruption if the data structure changes.
- You can safely update firmware or change configuration without risking old/bad data being loaded.

**NVS versioned settings include:**
- Pairing info
- Log level
- MIDI Program Change mapping

If you see a warning about an NVS version mismatch, the device has reset that setting to defaults for safety.

## Channel Select Mode (Unified Logic)

- **Entry:** Hold Button 1 for 15 seconds to enter channel select mode (works for both single and multi-button modes).
- **While in channel select mode:**
  - Each press of Button 1 increments the MIDI channel selection (cycles 1-16).
  - The LED flashes the number of times corresponding to the currently selected channel after each press.
  - No relay toggling or other actions occur while in channel select mode.
- **Auto-save:** After 10 seconds of inactivity, the selected channel is saved to NVS and the LED flashes the selected channel number as confirmation.
- **Exit:** Channel select mode exits automatically after auto-save.

### Example Flow
1. Hold Button 1 for 15s → Channel select mode active (LED fades)
2. Press Button 1 three times → LED flashes 3 times after each press (channel 3 selected)
3. Wait 10s → Channel 3 is saved, LED flashes 3 times as confirmation

## LED Feedback (Milestone Tracking)
- The LED flashes once at each 5s, 10s, 15s, 20s, and 25s milestone during a long press, providing clear timing feedback.
- This logic is now shared for both single and multi-button modes.

## Codebase Optimization
- The channel select and LED feedback logic is now unified and shared between single and multi-button modes, reducing code duplication and improving maintainability.

## Troubleshooting

- **Pins or config command does not match your expected configuration:**  
  Check your `platformio.ini` build flags and rebuild the firmware. All configuration is dynamic and reported at runtime.
- **MIDI Learn does not work:**  
  Ensure you are holding both Button 1 and Button 2 for >2s, then releasing and pressing the desired channel button, and then sending a MIDI Program Change. Use `midimap` to verify the current mapping.
- **No serial output:**  
  Check your serial monitor baud rate (115200), verify your USB connection, and try a different USB cable if needed.
- **Pairing issues:**  
  Use the `pair` command to reset pairing. Check WiFi channel compatibility and ensure both devices are in pairing mode.
- **OTA not working:**  
  Enter OTA mode by holding Button 1 for 5s during the setup window after boot, or by sending the `ota` serial command.
- **Memory issues:**  
  Use `debugmemory` to check memory usage. Monitor for memory leaks and consider reducing log level if needed.
- **MIDI not working:**  
  Check MIDI cable connections, verify MIDI channel settings, and test with the `midi` command.
- **Log level or debug output missing:**  
  Use `setlog4` to enable all logs, or `loglevel` to check the current log level.
- **NVS version mismatch warning:**  
  The device has reset that setting to defaults for safety. This is normal after a firmware upgrade or config structure change.
