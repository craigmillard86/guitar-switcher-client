# ESP32 Guitar Amp Channel Switcher

A wireless MIDI-controlled amplifier channel switcher for guitarists. Switch amp channels via MIDI Program Change messages or physical buttons, with wireless remote control capability.

## What It Does

- **Switch amp channels** using MIDI Program Change messages from your controller/pedalboard
- **Physical button control** for direct channel switching without MIDI
- **Learn MIDI mappings** - assign any MIDI Program Change number to any amp channel
- **Wireless control** via ESP-NOW for remote switching
- **Visual feedback** with status LED for all operations

## Quick Start

### 1. Hardware Setup
Connect your ESP32 with:
- **Relay** on pin 2 (controls amp channel switching)
- **Button** on pin 1 (manual channel control)  
- **LED** on pin 8 (status feedback)
- **MIDI IN** on pin 6 (from your MIDI controller)

> **Note:** Current configuration is single-channel mode. Multi-channel setup available via build flags.

### 2. Basic Operation

**Switch Channels:**
- **MIDI**: Send Program Change messages on MIDI channel 1 (default)
- **Button**: Press button to toggle relay ON/OFF (single-channel mode)
- **Serial**: Type `1` in serial monitor to toggle channel

**Check Status:**
- Type `status` in serial monitor to see current channel and MIDI settings

## MIDI Learn - Map Your Controller

Map any MIDI Program Change number to any amp channel:

### Single Channel Mode:
1. **Hold Button 1 for 10+ seconds** → Release (LED blinks fast)
2. **Send a Program Change** from your MIDI controller
3. **Done!** That PC number now controls the channel

### Multi-Channel Mode:
1. **Hold Button 1 for 10+ seconds** → Release (LED blinks fast)
2. **Press the channel button** you want to program (1-4)
3. **Send a Program Change** from your MIDI controller  
4. **Done!** That PC number now switches to that channel

**View Your Mappings:** Type `midimap` in serial monitor

**Example Mapping:**
```
Channel 1: PC#0 (Clean)
Channel 2: PC#1 (Crunch) 
Channel 3: PC#2 (Lead)
Channel 4: PC#3 (Solo Boost)
```

## Other Functions

### Change MIDI Channel (1-16)
1. **Hold Button 1 for 15+ seconds** → Release (LED fades)
2. **Press Button 1** repeatedly to cycle through channels 1-16
3. **Wait 10 seconds** - LED flashes to confirm the selected channel

### Pairing Mode (for wireless remotes)
1. **Hold Button 1 for 30+ seconds** → Release
2. LED fades - device is ready to pair with wireless remotes

### Firmware Updates (OTA)
1. **During first 10 seconds after power-on**: Hold Button 1 for 5+ seconds
2. Connect to WiFi network `ESP32_OTA` (password: `12345678`)
3. Go to `http://192.168.4.1/update` and upload new firmware

## Serial Commands

Connect via USB and open serial monitor (115200 baud) for these commands:

> **📋 For complete command reference, see [COMMANDS_REFERENCE.md](COMMANDS_REFERENCE.md)**

### Essential Commands
- `status` - Show current channel and MIDI settings
- `midimap` - Show Program Change to channel mappings
- `1`, `2`, `3`, `4` - Switch to channel 1-4
- `pins` - Show current pin assignments
- `help` - Show all available commands

### Configuration Commands  
- `setlog4` - Enable detailed logging
- `clearall` - Reset all settings to defaults
- `restart` - Reboot device
- `ota` - Enter firmware update mode

## LED Status Indicators

- **Single Flash**: Button timing feedback, MIDI learned successfully
- **Triple Flash**: MIDI message received, switching channels
- **Fast Blink**: MIDI Learn mode active (waiting for your controller)
- **Fade/Breath**: Channel select mode or pairing mode active
- **OFF**: Normal operation

## Troubleshooting

**MIDI not working?**
- Check MIDI cable connections
- Verify you're sending Program Change (not Note On/Off)
- Check MIDI channel setting (type `midi` to see current channel)

**Can't learn MIDI?**
- Make sure you hold Button 1 for full 10+ seconds before releasing
- Wait for LED to blink fast before sending Program Change
- Try `clearall` to reset if mappings seem stuck

**Buttons not working?**
- Check connections to pins 1, 3, 4, 5
- Type `buttons` to toggle button checking on/off

**Need help?**
- Type `help` in serial monitor for complete command list
- Type `debug` for detailed system information

---

## Build Configuration & Performance Modes

### Three Performance Modes

The system supports three distinct performance modes optimized for different use cases:

#### 1. Ultra-Fast Single Channel Mode
**Configuration:** `MAX_AMPSWITCHS=1` + `FAST_SWITCHING=1`
- **Switching Speed:** 5-30 microseconds (0.005-0.030ms)
- **Method:** Direct GPIO register writes (`REG_WRITE`)
- **Logging:** Disabled for maximum performance
- **Use Case:** Professional guitar performance requiring instant channel switching
- **Latency:** Sub-millisecond response time

#### 2. Standard Single Channel Mode  
**Configuration:** `MAX_AMPSWITCHS=1` (without `FAST_SWITCHING`)
- **Switching Speed:** ~500 microseconds (0.5ms)
- **Method:** Standard Arduino `digitalWrite()`
- **Logging:** Full logging and validation enabled
- **Use Case:** Development, debugging, or when logging is needed
- **Latency:** Human-imperceptible but measurable delay

#### 3. Multi-Channel Mode
**Configuration:** `MAX_AMPSWITCHS=2-4` (optionally with `FAST_SWITCHING`)
- **Fast Mode:** 10-50 microseconds per channel with `FAST_SWITCHING=1`
- **Standard Mode:** ~2-5ms total switching time without `FAST_SWITCHING`
- **Method:** Either register writes or digitalWrite() for all channels
- **Features:** Full channel validation, bounds checking, comprehensive logging
- **Use Case:** Multi-channel amps requiring exclusive channel selection

### Current Build Configuration
The device is configured via build flags in `platformio.ini`:

```ini
[env:esp32-c3-devkitc-02]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
build_flags = 
    -D CLIENT_TYPE=AMP_SWITCHER
    -D MAX_AMPSWITCHS=1                    # Single channel mode
    -D AMP_SWITCH_PINS=\"4\"               # Single relay pin
    -D AMP_BUTTON_PINS=\"1\"               # Single button pin
    -D DEVICE_NAME=\"AMP_SWITCHER_1\"
    -D FAST_SWITCHING=1                    # Enable ultra-fast mode
```

### Switching to Different Modes

**For Ultra-Fast Single Channel (Current):**
```ini
-D MAX_AMPSWITCHS=1
-D FAST_SWITCHING=1
-D AMP_SWITCH_PINS=\"4\"
-D AMP_BUTTON_PINS=\"1\"
```

**For Standard Single Channel:**
```ini
-D MAX_AMPSWITCHS=1
# Remove or comment out FAST_SWITCHING
-D AMP_SWITCH_PINS=\"4\" 
-D AMP_BUTTON_PINS=\"1\"
```

**For Ultra-Fast Multi-Channel:**
```ini
-D MAX_AMPSWITCHS=4
-D FAST_SWITCHING=1
-D AMP_SWITCH_PINS=\"2,9,10,20\"
-D AMP_BUTTON_PINS=\"1,3,4,5\"
```

**For Standard Multi-Channel:**
```ini
-D MAX_AMPSWITCHS=4
# Remove or comment out FAST_SWITCHING
-D AMP_SWITCH_PINS=\"2,9,10,20\"
-D AMP_BUTTON_PINS=\"1,3,4,5\"
```

### Performance Testing

Test switching performance using the serial command:
- `speed` - Measures actual switching time in microseconds

**Expected Performance:**
- **Ultra-Fast Mode:** 5-30μs switching time
- **Standard Mode:** 500μs-5ms switching time  
- **Logging Impact:** Serial logging adds ~500μs delay

### Customizing Hardware Configuration

**To change pin assignments or channel count:**
1. Edit the build flags in `platformio.ini`
2. Rebuild and flash the firmware  
3. Use `pins` and `config` commands to verify new settings

**Available Build Flags:**
- `MAX_AMPSWITCHS` - Number of channels (1-4)
- `FAST_SWITCHING` - Enable ultra-fast GPIO register access (optional)
- `AMP_SWITCH_PINS` - Relay control pins (comma-separated)
- `AMP_BUTTON_PINS` - Button input pins (comma-separated)
- `DEVICE_NAME` - Device identifier
- `PAIRING_LED_PIN` - Status LED pin (default: 8)
- `MIDI_RX_PIN` - MIDI input pin (default: 6)
- `MIDI_TX_PIN` - MIDI output pin (default: 7)

### Technical Architecture

**Button Handling System:**
- Unified logic for single and multi-button configurations
- Release-based activation for all long press functions
- Configurable debounce timing (100ms default)
- Milestone LED feedback at 5s intervals

**MIDI System:**
- 30-second learn timeout with automatic exit
- 2-second cooldown after learn completion
- NVS persistence for all mappings
- Support for MIDI channels 1-16

**Storage Management:**
- NVS (Non-Volatile Storage) with version control
- Automatic migration/reset on version mismatch
- Centralized storage in `nvsManager.h/cpp`
- Settings: MIDI maps, channel, log level, pairing

**Wireless Communication:**
- ESP-NOW protocol for low-latency control
- Automatic pairing system
- MAC address management
- Channel conflict resolution

### Serial Command Reference

**System Information:**
- `status` - Complete system status
- `config` - Build configuration details
- `pins` - Runtime pin assignments
- `version` - Firmware and storage versions
- `uptime` - System uptime
- `memory` - Memory usage statistics

**MIDI Commands:**
- `midimap` - Show PC to channel mappings
- `midi` - Show MIDI configuration
- `1-4` - Switch to specific channel
- `off` - Turn all channels off

**Debug Commands:**
- `debug` - Complete system debug info
- `debugperf` - Performance metrics
- `debugmemory` - Memory analysis
- `debugespnow` - Wireless statistics
- `setlog0-4` - Set logging level

**Maintenance Commands:**
- `clearall` - Reset all NVS settings
- `clearlog` - Reset log level only
- `pair` - Clear pairing and re-pair
- `buttons` - Toggle button checking
- `restart` - System reboot
- `ota` - Enter OTA update mode

### Hardware Technical Specifications

**ESP32-C3 Requirements:**
- Minimum 4MB flash recommended
- USB-C for programming and serial
- 3.3V I/O levels for all connections

**Pin Configuration:**
- **GPIO 8**: Reserved for status LED (PWM controlled)
- **GPIO 1,3,4,5**: Button inputs (internal pull-up enabled)
- **GPIO 2,9,10,20**: Relay outputs (active HIGH, 3.3V)
- **GPIO 6**: MIDI RX (5V tolerant via optocoupler)
- **GPIO 7**: MIDI TX (current source via 220Ω resistor)

**Power Requirements:**
- ESP32: ~80mA @ 3.3V during operation
- Relays: Varies by type (typically 10-70mA each @ 3.3V-12V)
- Total system: Plan for 200-500mA depending on relay configuration

**MIDI Interface:**
- Standard 5-pin DIN MIDI IN connection
- Optocoupler isolation recommended (6N138 or H11L1)
- MIDI THRU capability on TX pin
- Supports MIDI 1.0 specification

### LED Status Pattern Details

**Single Flash Patterns:**
- Button milestone feedback (5s, 10s, 15s, 20s, 25s)
- MIDI Learn completion confirmation
- ESP-NOW data received indication

**Multi-Flash Patterns:**
- **Double Flash**: Serial command received
- **Triple Flash**: MIDI Program Change received and processed
- **Quadruple Flash**: Channel switching confirmation

**Continuous Patterns:**
- **Fast Blink (200ms)**: MIDI Learn mode armed
- **Slow Blink (1000ms)**: OTA update mode
- **Fade/Breath**: Channel select or pairing mode
- **Solid ON**: System error state
- **OFF**: Normal operation

### NVS Storage Layout

**Storage Namespaces:**
- `pairing` - ESP-NOW MAC addresses and channels
- `midi` - Program Change mappings and MIDI channel
- `system` - Log level and configuration flags

**Storage Versioning:**
- Current version tracked in `STORAGE_VERSION` constant
- Automatic reset to defaults on version mismatch
- Safe firmware upgrades without data corruption
- Manual override with `clearall` command

**Data Persistence:**
- MIDI mappings survive power cycles
- Channel preferences retained
- Pairing information persistent
- Log level settings maintained
