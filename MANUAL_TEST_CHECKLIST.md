# ESP32 Amp Channel Switcher – Manual Test Checklist (Detailed)

---

## 1. Boot & Basic Functionality

### 1.1 Power-On and Serial Output
- [ ] **Step 1:** Connect the ESP32 to your computer via USB.
- [ ] **Step 2:** Open a serial monitor at 115200 baud.
- [ ] **Step 3:** Power on or reset the device.
- [ ] **Expect:**
  - Serial output includes:
    - `[INFO] === ESP32 Client Starting ===`
    - Firmware version, board ID, pin assignments
    - `[INFO] Type 'help' for available commands`
  - No `[ERROR]` or `[WARN]` messages on boot.
- [ ] **Check:**
  - All pin numbers match your hardware and config.
  - Device does not reboot unexpectedly.
- [ ] **If Fail:**
  - Record the serial output and any error messages.
  - Note if the device is stuck, reboots, or pins are incorrect.

### 1.2 Help and Status Commands
- [ ] **Step 1:** In the serial monitor, type `help` and press Enter.
- [ ] **Step 2:** Type `status` and press Enter.
- [ ] **Expect:**
  - `help` prints a full command menu.
  - `status` prints:
    - Firmware version, uptime, memory, amp channel, pairing status, etc.
- [ ] **If Fail:**
  - Record the output and which fields are missing or incorrect.

---

## 2. Button Functionality

### 2.1 Channel Switching (All Buttons)
- [ ] **Step 1:** Press and release Button 1.
- [ ] **Step 2:** Observe the relay/amp channel and LED.
- [ ] **Step 3:** Check serial log for:
  - `[INFO] Switching amp channel from X to 1`
  - `[INFO] Amp channel 1 activated (pin Y)`
- [ ] **Step 4:** Repeat for Buttons 2, 3, and 4.
- [ ] **Expect:**
  - Only one relay/channel is active at a time.
  - LED gives triple flash or as documented.
  - Channel only switches on button release.
- [ ] **If Fail:**
  - Note which button failed, what the serial log showed, and LED behavior.

### 2.2 Button 1 Long Press (Pairing Mode)
- [ ] **Step 1:** After boot/setup window, press and hold Button 1 for >5 seconds.
- [ ] **Step 2:** Observe LED pattern during hold and after release.
- [ ] **Step 3:** Check serial log for:
  - `[INFO] Long press detected on Button 1: Pairing mode triggered!`
- [ ] **Expect:**
  - LED enters fade/breath pattern.
  - No channel switching occurs.
- [ ] **If Fail:**
  - Note the duration held, LED pattern, and serial output.

### 2.3 Button 1 + Button 2 (MIDI Learn Arm)
- [ ] **Step 1:** Press and hold Button 1 and Button 2 together for >2 seconds.
- [ ] **Step 2:** Observe LED pattern during hold and after release.
- [ ] **Step 3:** Check serial log for:
  - `[INFO] MIDI Learn mode armed. Release and press a channel button to select.`
- [ ] **Expect:**
  - LED blinks fast (MIDI Learn pattern).
  - No channel switching occurs.
- [ ] **If Fail:**
  - Note the duration held, LED pattern, and serial output.

### 2.4 Channel Button Release in MIDI Learn
- [ ] **Step 1:** After arming MIDI Learn, release both buttons.
- [ ] **Step 2:** Short-press and release a channel button (e.g., Button 3).
- [ ] **Step 3:** Observe LED and serial log for:
  - `[INFO] MIDI Learn: Waiting for MIDI PC for channel 3`
  - LED triple flash.
- [ ] **Expect:**
  - No channel switching occurs.
  - Device is now waiting for MIDI PC.
- [ ] **If Fail:**
  - Note which button was pressed, serial output, and LED pattern.

---

## 3. MIDI Learn Feature

### 3.1 MIDI Learn Mapping
- [ ] **Step 1:** Arm MIDI Learn and select a channel (see 2.3, 2.4).
- [ ] **Step 2:** On your MIDI controller, send a Program Change (PC) message.
- [ ] **Step 3:** Observe serial log for:
  - `[INFO] MIDI PC#X assigned to channel Y`
  - LED single flash.
- [ ] **Step 4:** Try sending the same PC again; device should switch to the mapped channel.
- [ ] **Expect:**
  - Mapping is saved and used for channel switching.
- [ ] **If Fail:**
  - Note the PC number sent, serial output, and channel behavior.

### 3.2 MIDI Learn Timeout
- [ ] **Step 1:** Arm MIDI Learn, select a channel, do NOT send a PC for 30 seconds.
- [ ] **Step 2:** Observe serial log for:
  - `[WARN] MIDI Learn timed out, exiting learn mode.`
- [ ] **Step 3:** After timeout, try pressing buttons and sending serial commands.
- [ ] **Expect:**
  - No channel switching or serial commands are accepted during lockout.
  - After timeout, normal operation resumes.
- [ ] **If Fail:**
  - Note the time waited, serial output, and device behavior.

### 3.3 MIDI Learn Lockout
- [ ] **Step 1:** While waiting for MIDI PC, press any channel button or send serial commands.
- [ ] **Expect:**
  - No channel switching or serial command response.
  - Device only responds to MIDI PC or timeout.
- [ ] **If Fail:**
  - Note which actions were attempted and any unexpected behavior.

### 3.4 MIDI Mapping Persistence
- [ ] **Step 1:** Power cycle the device.
- [ ] **Step 2:** Type `midimap` in serial.
- [ ] **Expect:**
  - Mapping from previous MIDI Learn is still present.
  - Serial log shows correct PC numbers for each channel.
- [ ] **If Fail:**
  - Note the mapping before and after reboot.

---

## 4. Serial Command Interface

### 4.1 Pin and Config Reporting
- [ ] **Step 1:** Type `pins` in serial, press Enter.
- [ ] **Step 2:** Type `config` in serial, press Enter.
- [ ] **Expect:**
  - Pin assignments and configuration match your hardware and build flags.
  - Serial output is formatted and complete.
- [ ] **If Fail:**
  - Note which fields are missing or incorrect.

### 4.2 All Serial Commands
- [ ] **Step 1:** For each command in the help menu, type the command and press Enter.
- [ ] **Step 2:** Observe serial output and device behavior.
- [ ] **Expect:**
  - Each command produces the expected output or action.
  - No command causes a crash, hang, or unexpected behavior.
- [ ] **If Fail:**
  - Note the command, output, and any issues.

---

## 5. OTA Update (Access Point + ElegantOTA)

### 5.1 Enter OTA Mode via Serial
- [ ] **Step 1:** Type `ota` in serial, press Enter.
- [ ] **Step 2:** Observe serial log for:
  - `[INFO] OTA mode triggered, starting OTA...`
- [ ] **Step 3:** Scan for WiFi networks; confirm `ESP32_OTA` appears.
- [ ] **Step 4:** Observe LED blinks fast (OTA pattern).
- [ ] **If Fail:**
  - Note serial output, AP visibility, and LED pattern.

### 5.2 Enter OTA Mode via Button
- [ ] **Step 1:** Hold Button 1 for 5s during boot/setup window.
- [ ] **Step 2:** Observe same as above.
- [ ] **If Fail:**
  - Note timing, serial output, and AP visibility.

### 5.3 Connect and Upload Firmware
- [ ] **Step 1:** Connect to `ESP32_OTA` WiFi (password: `12345678`).
- [ ] **Step 2:** Open browser to `http://192.168.4.1/update`.
- [ ] **Step 3:** Upload a new firmware file.
- [ ] **Step 4:** Observe progress bar, “Update Received: firmware.bin”, and device reboot.
- [ ] **Step 5:** After reboot, check serial log for new firmware version.
- [ ] **If Fail:**
  - Note browser output, serial log, and device behavior.

### 5.4 OTA Timeout
- [ ] **Step 1:** Enter OTA mode, do NOT upload firmware for 5 minutes.
- [ ] **Step 2:** Observe serial log for:
  - `[INFO] OTA timeout reached, rebooting...`
- [ ] **Step 3:** Device reboots.
- [ ] **If Fail:**
  - Note time waited, serial output, and device behavior.

---

## 6. LED Feedback Patterns

### 6.1 Single, Double, Triple Flash
- [ ] **Step 1:** Trigger each event:
  - Single: Simulate ESP-NOW data (or use a test command).
  - Double: Send a serial command.
  - Triple: Send a MIDI message.
- [ ] **Step 2:** Observe LED for correct flash pattern.
- [ ] **If Fail:**
  - Note which pattern failed and what was observed.

### 6.2 Pairing and OTA Patterns
- [ ] **Step 1:** Enter pairing mode (Button 1 long press) and OTA mode (serial or button).
- [ ] **Step 2:** Observe LED for fade (pairing) and fast blink (OTA).
- [ ] **If Fail:**
  - Note which pattern failed and what was observed.

### 6.3 Error Pattern
- [ ] **Step 1:** Send an invalid command or cause an error.
- [ ] **Step 2:** Observe LED for solid on (if implemented).
- [ ] **If Fail:**
  - Note error type and LED behavior.

---

## 7. NVS Storage and Versioning

### 7.1 Log Level Persistence
- [ ] **Step 1:** Type `setlog4` (or another level), press Enter.
- [ ] **Step 2:** Reboot device.
- [ ] **Step 3:** Type `loglevel` in serial.
- [ ] **Expect:**
  - Log level is retained after reboot.
- [ ] **If Fail:**
  - Note log level before and after reboot.

### 7.2 NVS Version Mismatch
- [ ] **Step 1:** Flash firmware with a different `STORAGE_VERSION`.
- [ ] **Step 2:** Observe serial log for:
  - `[WARN] NVS version mismatch... settings reset to defaults.`
- [ ] **If Fail:**
  - Note serial output and settings after upgrade.

---

## 8. Edge Cases & Robustness

### 8.1 Button Bounce
- [ ] **Step 1:** Rapidly press and release each button 10+ times.
- [ ] **Step 2:** Observe serial log and channel switching.
- [ ] **Expect:**
  - No false triggers or missed presses (debounce works).
- [ ] **If Fail:**
  - Note which button, how many presses, and any missed/extra triggers.

### 8.2 Simultaneous Events
- [ ] **Step 1:** Try to trigger pairing, MIDI Learn, and OTA in quick succession (e.g., hold Button 1, then Button 2, then send `ota` command).
- [ ] **Step 2:** Observe which mode is active and serial log.
- [ ] **Expect:**
  - Only one mode is active at a time, no lockups.
- [ ] **If Fail:**
  - Note sequence, serial output, and device state.

### 8.3 Low Memory
- [ ] **Step 1:** Type `debugmemory` in serial.
- [ ] **Step 2:** Observe serial log for low memory warnings.
- [ ] **If Fail:**
  - Note memory values and any warnings.

---

## Channel Select Mode (Unified)
- [ ] Hold Button 1 for 15 seconds. LED should fade to indicate channel select mode is active.
- [ ] Press Button 1 repeatedly. Each press increments the MIDI channel (cycles 1-16), and the LED flashes the selected channel number after each press.
- [ ] No relay toggling or other actions should occur while in channel select mode.
- [ ] Wait 10 seconds without pressing any button. The selected channel should be saved, and the LED should flash the selected channel number as confirmation.
- [ ] Channel select mode should exit automatically after auto-save.

## LED Feedback (Milestone Tracking)
- [ ] During any long press, the LED should flash once at each 5s, 10s, 15s, 20s, and 25s milestone.
- [ ] This feedback should work identically in both single and multi-button modes.

---

# Test Completion Checklist

- [ ] **Boot & Serial Output**
- [ ] **All Buttons (short/long press, all channels)**
- [ ] **MIDI Learn (arm, map, timeout, lockout, persistence)**
- [ ] **Serial Commands (all)**
- [ ] **OTA Update (serial, button, upload, timeout)**
- [ ] **LED Patterns (all)**
- [ ] **NVS Storage (log level, mapping, versioning)**
- [ ] **Edge Cases (debounce, simultaneous, low memory)**

---

**Instructions:**
- For each test, check the box when complete.
- Record any unexpected behavior, serial output, or LED patterns.
- If a test fails, note the steps and output for debugging. 