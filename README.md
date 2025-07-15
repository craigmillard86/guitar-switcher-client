# ESP32 Amp Channel Switcher

A sophisticated ESP32-based amplifier channel switcher with wireless control, MIDI support, and comprehensive logging capabilities. Supports multiple amp configurations through build-time configuration.

## Features

- **Multi-Channel Amp Switching**: Support for 2-4 amplifier channels via relays
- **Build-Time Configuration**: Multiple configurations for different amps and channel counts, controlled by the `CLIENT_TYPE` and other build flags
- **Wireless Control**: ESP-NOW communication for wireless remote control
- **MIDI Support**: Program change messages for channel switching
- **OTA Updates**: Over-the-air firmware updates
- **Comprehensive Logging**: Multi-level logging with timestamps and NVS persistence
- **Serial Commands**: Extensive command interface for debugging and control
- **Performance Monitoring**: Real-time performance and memory tracking
- **Auto-Pairing**: Automatic device pairing system
- **Button Support**: Physical buttons for direct channel switching

## Build-Time Configuration System

All hardware pin assignments, feature flags, and channel counts are now set in `config.h` or via build flags in `platformio.ini`. The `CLIENT_TYPE` macro determines the device's role and features at build time.

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

## Hardware Setup

### Pin Configuration
- **Amp Switch Pins**: GPIO 2, 3, 4, 5 (relay control)
- **Amp Button Pins**: GPIO 8, 9, 10, 20 (physical buttons)
- **Status/Pairing LED**: GPIO 1 (PWM, reserved for LED only)
- **MIDI RX**: GPIO 6
- **MIDI TX**: GPIO 7

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

## Enhanced Logging System

### Log Levels
- **LOG_NONE (0)**: No logging
- **LOG_ERROR (1)**: Error messages only
- **LOG_WARN (2)**: Warnings and errors
- **LOG_INFO (3)**: Information, warnings, and errors
- **LOG_DEBUG (4)**: All messages including debug

### Log Format
```
[MM:SS][LEVEL] Message
```

Example:
```
[00:15][INFO] Amp channel set to 2
[00:16][DEBUG] Button 3 pressed, switching to channel 3
[00:17][WARN] Low memory warning: 8500B free
```

### Log Persistence
- Log levels are saved to NVS (Non-Volatile Storage)
- Settings persist across reboots
- Commands to manage log levels:
  - `setlogN` - Set log level (N=0-4)
  - `loglevel` - Show current log level
  - `clearlog` - Reset to default log level

## Serial Commands

### System Commands
| Command | Description |
|---------|-------------|
| `help` | Show complete help menu |
| `status` | Show complete system status |
| `config` | Show client configuration |
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

## Examples

```bash
# Set log level to show info and above
setlog3

# Switch to channel 2
2

# Simulate button 3 press
b3

# Show system status
status

# Show client configuration
config

# Show debug information
debug

# Test status LED
testled

# Clear all NVS data
clearall
```

## Performance Monitoring

The system includes comprehensive performance monitoring:

- **Loop Performance**: Tracks loop execution time
- **Memory Usage**: Monitors heap memory usage
- **Memory Leak Detection**: Alerts on potential memory leaks
- **WiFi Statistics**: RSSI, channel, power mode
- **ESP-NOW Statistics**: Peer count, pairing status
- **Task Statistics**: Stack usage, CPU usage

## Memory Management

- **Initial Memory**: Tracks initial free heap
- **Memory Changes**: Logs memory increases/decreases
- **Low Memory Warnings**: Alerts when free memory < 10KB
- **Memory Leak Detection**: Compares current vs initial memory

## MIDI Integration

- **Program Changes**: MIDI PC messages switch amp channels
- **Channel Mapping**: PC#0 = Channel 1, PC#1 = Channel 2, etc.
- **MIDI Thru**: Passes incoming MIDI to output
- **OMNI Mode**: Listens to all MIDI channels

## ESP-NOW Communication

- **Auto-Pairing**: Automatic device discovery and pairing
- **Channel Selection**: Automatic WiFi channel selection
- **Peer Management**: Dynamic peer addition/removal
- **Message Types**: DATA, PAIRING, COMMAND

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
    -D AMP_SWITCH_PINS=\"4,5\"
    -D AMP_BUTTON_PINS=\"8,9\"
    -D DEVICE_NAME=\"2CH_AMP\"
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
    -D AMP_SWITCH_PINS=\"4,5,6\"
    -D AMP_BUTTON_PINS=\"8,9,10\"
    -D DEVICE_NAME=\"CUSTOM_AMP\"
```

2. Build with the new configuration:
```bash
platformio run -e client-custom-amp
```

## Troubleshooting

### Common Issues

1. **No Serial Output**
   - Check baud rate (115200)
   - Verify USB connection
   - Try different USB cable

2. **Pairing Issues**
   - Use `pair` command to reset pairing
   - Check WiFi channel compatibility
   - Verify both devices are in pairing mode

3. **Memory Issues**
   - Use `debugmemory` to check memory usage
   - Monitor for memory leaks
   - Consider reducing log level

4. **MIDI Not Working**
   - Check MIDI cable connections
   - Verify MIDI channel settings
   - Test with `midi` command

5. **Wrong Configuration**
   - Use `config` command to verify current configuration
   - Check build environment matches your hardware
   - Rebuild with correct configuration

### Debug Commands

Use these commands for troubleshooting:

```bash
# Check system status
status

# Check client configuration
config

# Monitor memory usage
debugmemory

# Check WiFi connection
debugwifi

# Monitor performance
debugperf

# Check ESP-NOW status
debugespnow
```

## Version History

- **v1.0.0**: Initial release with basic functionality
- **v1.1.0**: Enhanced logging and serial commands
- **v1.2.0**: Performance monitoring and debug features
- **v1.3.0**: Multi-configuration build system and NVS persistence

## License

This project is open source. Feel free to modify and distribute.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review the debug commands
3. Check the serial output for error messages
4. Use the `help` command for available options 