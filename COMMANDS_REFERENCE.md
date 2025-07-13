# ESP32 Amp Switcher - Quick Command Reference

## Essential Commands

| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `status` | Show complete system status |
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

## Amp Channel Control

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
| `ota` | Enter OTA update mode |
| `pair` | Clear pairing and re-pair |

## Common Usage Examples

```bash
# Set logging to show info and above
setlog3

# Check system status
status

# Switch to channel 2
2

# Show debug info
debug

# Test LED
testled

# Check memory usage
debugmemory

# Restart device
restart
```

## Quick Troubleshooting

1. **No response from device**: Try `restart`
2. **Can't see logs**: Try `setlog4`
3. **Memory issues**: Use `debugmemory`
4. **WiFi problems**: Use `debugwifi`
5. **Pairing issues**: Use `pair` then check `debugespnow`

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
``` 