# Logging Improvements Summary

## Overview
This document summarizes all the logging improvements made to the ESP32 Amp Channel Switcher project to ensure consistent, professional logging throughout the codebase with NVS persistence and configuration system integration.

## Key Improvements Made

### 1. **Consistent Log Format**
- **Before**: Mixed formats like `[Client]`, `[NVS]`, `Serial.println()`
- **After**: Unified format `[MM:SS][LEVEL] Message`
- **Example**: `[00:15][INFO] Amp channel set to 2`

### 2. **Proper Log Levels**
- **LOG_NONE (0)**: No logging
- **LOG_ERROR (1)**: Critical errors only
- **LOG_WARN (2)**: Warnings and errors
- **LOG_INFO (3)**: Normal operation information
- **LOG_DEBUG (4)**: Detailed debug information

### 3. **NVS Log Level Persistence**
- **Log levels saved to NVS**: Settings persist across reboots
- **Version control**: Storage version checking for compatibility
- **Commands**: `setlogN`, `loglevel`, `clearlog`, `clearall`
- **Default fallback**: LOG_INFO if NVS unavailable

### 4. **Configuration System Integration**
- **Build-time configuration**: Multiple amp configurations
- **Config command**: `config` shows current client configuration
- **Device-specific logging**: Different configurations for different amps
- **Pin assignment logging**: Shows configured pins for current setup

### 5. **Files Updated**

#### `src/main.cpp`
- ✅ Added proper initialization logging
- ✅ Used correct log levels (INFO for startup, DEBUG for details)
- ✅ Added performance monitoring
- ✅ Enhanced error handling
- ✅ Added client configuration initialization

#### `src/utils.cpp`
- ✅ Complete rewrite with enhanced logging system
- ✅ Added timestamps and log level formatting
- ✅ Comprehensive serial command handling
- ✅ System status and debug functions
- ✅ Memory tracking and performance monitoring
- ✅ NVS log level persistence functions
- ✅ Configuration display functions

#### `src/commandHandler.cpp`
- ✅ Updated amp channel operations with proper logging
- ✅ Added MIDI event logging
- ✅ Enhanced error handling for invalid channels
- ✅ Debug logging for button presses

#### `src/espnow.cpp`
- ✅ Improved ESP-NOW event logging
- ✅ Better error handling and status reporting
- ✅ Enhanced packet reception logging
- ✅ Proper pairing status logging

#### `src/espnow-pairing.cpp`
- ✅ Updated NVS operations with proper log levels
- ✅ Enhanced pairing process logging
- ✅ Better error handling for peer management
- ✅ Improved timeout and retry logging

#### `src/otaManager.cpp`
- ✅ Added proper OTA mode logging
- ✅ Enhanced WiFi connection logging
- ✅ Better timeout and reboot logging
- ✅ Web interface event logging

#### `src/debug.cpp` (New)
- ✅ Performance monitoring system
- ✅ Memory leak detection
- ✅ WiFi and ESP-NOW statistics
- ✅ Task and system monitoring

#### `src/config.cpp` (New)
- ✅ Client configuration management
- ✅ Pin array parsing and initialization
- ✅ Configuration display functions
- ✅ Build-time configuration integration

### 6. **New Features Added**

#### Enhanced Logging Functions
```cpp
void log(LogLevel level, const String& msg);
void logf(LogLevel level, const char* format, ...);
void logWithTimestamp(LogLevel level, const String& msg);
```

#### NVS Log Level Management
```cpp
void saveLogLevelToNVS(LogLevel level);
LogLevel loadLogLevelFromNVS();
void clearLogLevelNVS();
```

#### System Status Functions
```cpp
void printSystemStatus();
void printMemoryInfo();
void printNetworkStatus();
void printAmpChannelStatus();
void printPairingStatus();
void printClientConfiguration();
```

#### Performance Monitoring
```cpp
void updatePerformanceMetrics(unsigned long loopTime);
void updateMemoryStats();
void printMemoryLeakInfo();
```

### 7. **Serial Commands Enhanced**

#### System Commands
- `help` - Complete help menu
- `status` - System status overview
- `config` - Show client configuration
- `memory` - Memory usage details
- `network` - WiFi status
- `amp` - Amp channel status
- `pairing` - Pairing status
- `uptime` - System uptime
- `version` - Firmware version
- `midi` - MIDI configuration
- `buttons` - Toggle button checking

#### Logging Commands
- `setlogN` - Set log level (N=0-4)
- `loglevel` - Show current log level
- `clearlog` - Clear saved log level
- `clearall` - Clear all NVS data

#### Debug Commands
- `debug` - Complete debug info
- `debugperf` - Performance metrics
- `debugmemory` - Memory analysis
- `debugwifi` - WiFi statistics
- `debugespnow` - ESP-NOW statistics
- `debugtask` - Task statistics

#### Control Commands
- `restart` - Reboot device
- `ota` - Enter OTA mode
- `pair` - Re-pair devices

### 8. **Log Level Usage Guidelines**

#### LOG_ERROR (1)
- Critical system failures
- Hardware errors
- Communication failures
- Memory allocation failures

#### LOG_WARN (2)
- Non-critical issues
- Timeout conditions
- Low memory warnings
- Retry attempts

#### LOG_INFO (3)
- Normal operation events
- State changes
- User actions
- System status updates

#### LOG_DEBUG (4)
- Detailed operation flow
- Performance metrics
- Memory usage details
- Packet information

### 9. **Memory Management**

#### Memory Tracking
- Initial memory baseline
- Real-time memory monitoring
- Memory leak detection
- Low memory warnings

#### Performance Monitoring
- Loop execution timing
- Memory usage tracking
- WiFi performance metrics
- Task statistics

### 10. **Configuration System**

#### Build-Time Configuration
- Multiple amp configurations (2ch, 4ch)
- Different pin assignments per configuration
- Device-specific names and settings
- PlatformIO build environment support

#### Configuration Display
- `config` command shows current setup
- Pin assignments and channel count
- Device name and client type
- Build-time configuration details

### 11. **Benefits of Improvements**

#### For Development
- **Easier Debugging**: Structured logs with timestamps
- **Better Error Tracking**: Proper error levels and messages
- **Performance Monitoring**: Real-time system metrics
- **Memory Management**: Leak detection and monitoring
- **Configuration Management**: Build-time configuration system

#### For Users
- **Clear Status**: Easy to understand system state
- **Troubleshooting**: Comprehensive debug commands
- **Remote Monitoring**: Detailed system information
- **Performance Insights**: Loop timing and memory usage
- **Persistent Settings**: Log levels saved across reboots
- **Configuration Awareness**: Know your current setup

#### For Maintenance
- **Consistent Format**: All logs follow same pattern
- **Configurable Levels**: Adjustable verbosity with persistence
- **Comprehensive Coverage**: All major operations logged
- **Error Recovery**: Better error handling and reporting
- **Multi-Configuration Support**: Different setups for different amps

### 12. **Usage Examples**

#### Setting Log Level
```bash
setlog3  # Show info and above
setlog4  # Show all messages including debug
setlog1  # Show only errors
loglevel # Check current level
clearlog # Reset to default
```

#### Configuration Management
```bash
config    # Show current configuration
clearall  # Reset all settings
```

#### Monitoring System
```bash
status    # Complete system overview
debug     # Detailed debug information
debugperf # Performance metrics
debugmemory # Memory analysis
```

#### Troubleshooting
```bash
# Check for issues
debugwifi
debugespnow
debugmemory

# Check configuration
config

# Reset everything
clearall
```

### 13. **Build Configurations**

Available build environments:
- `client-2ch-amp`: 2-channel amp switcher
- `client-4ch-amp`: 4-channel amp switcher  
- `client-amp-switcher`: Original 4-channel configuration

Build with: `platformio run -e <environment>`

The logging system has been completely overhauled to provide professional-grade debugging and monitoring capabilities with persistent settings and multi-configuration support while maintaining backward compatibility with existing functionality. 