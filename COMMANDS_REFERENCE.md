# ESP32 Amp Switcher - Quick Command Reference

> **Note:** The available commands, channel numbers, and button numbers depend on the build-time configuration (e.g., CLIENT_TYPE, MAX_AMPSWITCHS, pin assignments) set in platformio.ini and config.h. Use the `config` command to see your current build configuration.

## Essential Commands

| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `status` | Show complete system status |
| `config` | Show client configuration |
| `debug` | Show debug information |
| `restart` | Reboot the device |

## Logging Control

| Command | Description |
|---------|-------------|
| `setlog0` | Turn off all logging |
| `setlog1` | Show errors only |
| `setlog2` | Show warnings and errors |
| `setlog3` | Show info, warnings, and errors |
| `setlog4` | Show all messages (debug) |
| `loglevel` | Show current log level |
| `clearlog` | Clear saved log level (reset to default) |

## Amp Channel Control

| Command | Description |
|---------|-------------|
| `1` | Switch to channel 1 |
| `2` | Switch to channel 2 |
| `3` | Switch to channel 3 (4ch configs only) |
| `4` | Switch to channel 4 (4ch configs only) |
| `off` | Turn all channels off |
| `b1` | Simulate button 1 press |
| `b2` | Simulate button 2 press |
| `b3` | Simulate button 3 press (4ch configs only) |
| `b4` | Simulate button 4 press (4ch configs only) |

## System Information

| Command | Description |
|---------|-------------|
| `memory` | Show memory usage |
| `network` | Show WiFi status |
| `amp` | Show amp channel status |
| `pairing` | Show pairing status |
| `uptime` | Show system uptime |
| `version` | Show firmware version |
| `midi` | Show MIDI configuration |
| `buttons` | Toggle button checking on/off |

## Debug Commands

| Command | Description |
|---------|-------------|
| `debugperf` | Show performance metrics |
| `debugmemory` | Show memory analysis |
| `debugwifi` | Show WiFi statistics |
| `debugespnow` | Show ESP-NOW statistics |
| `debugtask` | Show task statistics |

## Testing Commands

| Command | Description |
|---------|-------------|
| `testled` | Test status LED |
| `testpairing` | Test pairing LED |

## Control Commands

| Command | Description |
|---------|-------------|
| `ota` | Enter OTA update mode (serial command, or hold Button 1 for 5s during setup) |
| `pair` | Clear pairing and re-pair |
| `clearall` | Clear all NVS data (pairing + log level) |

## Configuration Support & Pin Assignments

**4-Channel Example (default):**
- **Relay (Switch) Pins:** 2, 3, 4, 5
- **Button Pins:** 8, 9, 10, 20
- **Status/Pairing LED:** 1 (reserved for LED only)
- **MIDI RX/TX:** 6, 7

> **Note:** GPIO 1 is reserved for the status/pairing LED. Do **not** use GPIO 1 for relays or switches.
> **Note:** GPIO 20 is used for the 4th amp button on the ESP32-C3 Super Mini. GPIO 11 is not available on this board.

Use the `config` command to see your current build configuration and pin assignments.

## Common Usage Examples

```bash
# Set logging to show info and above
setlog3

# Check system status
status

# Check current configuration
config

# Switch to channel 2
2

# Show debug info
debug

# Test LED
testled

# Check memory usage
debugmemory

# Clear all settings
clearall

# Restart device
restart
```

## Quick Troubleshooting

1. **No response from device**: Try `restart`
2. **Can't see logs**: Try `setlog4`
3. **Memory issues**: Use `debugmemory`
4. **WiFi problems**: Use `debugwifi`
5. **Pairing issues**: Use `pair` then check `debugespnow`
6. **Wrong configuration**: Use `config` to verify settings
7. **Reset everything**: Use `clearall` to reset all settings

## Log Levels Explained

- **0 (OFF)**: No output
- **1 (ERROR)**: Only critical errors
- **2 (WARN)**: Warnings and errors
- **3 (INFO)**: Normal operation info
- **4 (DEBUG)**: Detailed debug info

## Performance Monitoring

Use these commands to monitor system health:

```bash
# Check overall performance
debugperf

# Monitor memory usage
debugmemory

# Check WiFi performance
debugwifi

# Monitor ESP-NOW status
debugespnow

# Check current configuration
config
```

## Button 1 Multi-Function Summary

| Action | When | Result |
|--------|------|--------|
| Short press (<5s) | Any time | Switch to channel 1 |
| Long press (>5s) | After setup window | Enter pairing mode |
| Long press (>5s) | During setup window (first 10s after boot) | Enter OTA mode |

## Build Configurations

Available build environments:
- `client-2ch-amp`: 2-channel amp switcher
- `client-4ch-amp`: 4-channel amp switcher  
- `client-amp-switcher`: Original 4-channel configuration

Build with: `platformio run -e <environment>` 