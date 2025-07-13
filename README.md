# ESP32 Amp Channel Switcher

A sophisticated ESP32-based amplifier channel switcher with wireless control, MIDI support, and comprehensive logging capabilities.

## Features

- **4-Channel Amp Switching**: Control up to 4 amplifier channels via relays
- **Wireless Control**: ESP-NOW communication for wireless remote control
- **MIDI Support**: Program change messages for channel switching
- **OTA Updates**: Over-the-air firmware updates
- **Comprehensive Logging**: Multi-level logging with timestamps
- **Serial Commands**: Extensive command interface for debugging and control
- **Performance Monitoring**: Real-time performance and memory tracking
- **Auto-Pairing**: Automatic device pairing system

## Hardware Setup

### Pin Configuration
- **Amp Switch Pins**: GPIO 4, 5, 6, 7 (relay control)
- **Amp Button Pins**: GPIO 4, 5, 6, 7 (physical buttons)
- **Status LED**: GPIO 2
- **Pairing LED**: GPIO 2 (PWM)
- **Pairing Button**: GPIO 0
- **MIDI RX**: GPIO 6
- **MIDI TX**: GPIO 7

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

## Serial Commands

### System Commands
| Command | Description |
|---------|-------------|
| `help` | Show complete help menu |
| `status` | Show complete system status |
| `memory` | Show memory usage |
| `network` | Show network status |
| `amp` | Show amp channel status |
| `pairing` | Show pairing status |
| `uptime` | Show system uptime |
| `version` | Show firmware version |
| `midi` | Show MIDI configuration |

### Control Commands
| Command | Description |
|---------|-------------|
| `restart` | Reboot the device |
| `ota` | Enter OTA update mode |
| `pair` | Clear pairing and re-pair |
| `setlogN` | Set log level (N=0-4) |

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

# Show debug information
debug

# Test status LED
testled
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

1. Install PlatformIO
2. Open the project in PlatformIO
3. Configure your board in `platformio.ini`
4. Build and upload:
   ```bash
   pio run --target upload
   ```

## Configuration

### Log Level
Set the default log level in `globals.cpp`:
```cpp
LogLevel currentLogLevel = LOG_DEBUG;
```

### Pin Configuration
Modify pin assignments in `globals.cpp`:
```cpp
uint8_t ampSwitchPins[MAX_AMPSWITCHS] = {4, 5, 6, 7};
uint8_t ampButtonPins[MAX_AMPSWITCHS] = {4, 5, 6, 7};
```

### Channel Count
Set the number of amp channels in `globals.h`:
```cpp
#define MAX_AMPSWITCHS 4
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

### Debug Commands

Use these commands for troubleshooting:

```bash
# Check system status
status

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