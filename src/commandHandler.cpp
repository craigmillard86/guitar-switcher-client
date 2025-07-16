// Copyright (c) Craig Millard and contributors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//
// See the License for the specific language governing permissions and
// limitations under the License.
#include <Arduino.h>
#include "config.h"
#include "commandHandler.h"
#include "pairing.h"
#include "globals.h"
#include "espnow-pairing.h"
#include "utils.h"
#include <Preferences.h>

void checkAmpChannelButtons() {
    // Skip button checking if disabled (when buttons aren't connected)
    if (!enableButtonChecking) {
        return;
    }
    
    static unsigned long lastDebounceTime[MAX_AMPSWITCHS] = {0};
    static uint8_t lastButtonState[MAX_AMPSWITCHS] = {HIGH};
    static bool buttonPressed[MAX_AMPSWITCHS] = {false};
    static unsigned long button1PressStart = 0;
    static bool button1LongPressHandled = false;
    const unsigned long debounceDelay = BUTTON_DEBOUNCE_MS;
    const unsigned long longPressTime = BUTTON_LONGPRESS_MS;

    // MIDI Learn chord detection
    static unsigned long midiLearnChordStart = 0;
    bool b1 = (digitalRead(ampButtonPins[0]) == LOW);
    bool b2 = (digitalRead(ampButtonPins[1]) == LOW);
    const unsigned long midiLearnChordTime = 2000; // 2 seconds

    if (b1 && b2) {
        if (midiLearnChordStart == 0) midiLearnChordStart = millis();
        if (!midiLearnArmed && (millis() - midiLearnChordStart > midiLearnChordTime)) {
            midiLearnArmed = true;
            log(LOG_INFO, "MIDI Learn mode armed. Release and press a channel button to select.");
            setStatusLedPattern(LED_FAST_BLINK);
        }
    } else {
        midiLearnChordStart = 0;
    }

    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        uint8_t reading = digitalRead(ampButtonPins[i]);
        
        // Only process if button state changed
        if (reading != lastButtonState[i]) {
            lastDebounceTime[i] = millis();
        }
        
        // Check if enough time has passed since last change
        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
            // MIDI Learn channel select
            if (midiLearnArmed && reading == LOW && !buttonPressed[i]) {
                midiLearnChannel = i;
                midiLearnArmed = false;
                log(LOG_INFO, String("MIDI Learn: Waiting for MIDI PC for channel ") + String(i+1));
                setStatusLedPattern(LED_TRIPLE_FLASH);
            }
            // Existing logic for pairing/OTA/normal operation
            if (i == 0) { // Button 1: support long press for pairing
                if (reading == LOW && !buttonPressed[i]) {
                    button1PressStart = millis();
                    buttonPressed[i] = true;
                    button1LongPressHandled = false;
                } else if (reading == LOW && buttonPressed[i]) {
                    // Still held
                    if (!button1LongPressHandled && (millis() - button1PressStart > longPressTime)) {
                        clearPairingNVS();
                        pairingStatus = NOT_PAIRED;
                        log(LOG_INFO, "Long press detected on Button 1: Pairing mode triggered!");
                        button1LongPressHandled = true;
                    }
                } else if (reading == HIGH && buttonPressed[i]) {
                    // Released
                    if (!button1LongPressHandled && (millis() - button1PressStart < longPressTime)) {
                        log(LOG_INFO, "Button 1 short press: switching to channel 1");
                        setAmpChannel(1);
                    }
                    buttonPressed[i] = false;
                    button1LongPressHandled = false;
                }
            } else {
                // Other buttons: normal short press logic
                if (reading == LOW && !buttonPressed[i] && currentAmpChannel != (i + 1)) {
                    log(LOG_INFO, "Button " + String(i + 1) + " pressed, switching to channel " + String(i + 1));
                    setAmpChannel(i + 1);
                    buttonPressed[i] = true; // Mark as pressed to prevent repeat
                } else if (reading == HIGH && buttonPressed[i]) {
                    buttonPressed[i] = false;
                }
            }
        }
        
        lastButtonState[i] = reading;
    }
}

void handleCommand(uint8_t commandType, uint8_t value) {
    log(LOG_DEBUG, "Received command - Type: " + String(commandType) + ", Value: " + String(value));
    
    switch (commandType) {
        case PROGRAM_CHANGE:
            log(LOG_INFO, "Program change command received: " + String(value));
            setAmpChannel(value);
            break;
        default:
            log(LOG_WARN, "Unknown command received - Type: " + String(commandType) + ", Value: " + String(value));
            break;
    }
}

void saveMidiMapToNVS() {
    Preferences nvs;
    if (nvs.begin("midi_map", false)) {
        nvs.putBytes("map", midiChannelMap, MAX_AMPSWITCHS);
        nvs.putInt("version", STORAGE_VERSION);
        nvs.end();
        log(LOG_INFO, "MIDI channel map saved to NVS");
    }
}

void loadMidiMapFromNVS() {
    Preferences nvs;
    if (nvs.begin("midi_map", true)) {
        if (nvs.getInt("version", 0) != STORAGE_VERSION) {
            // Version mismatch: reset to defaults and save
            for (int i = 0; i < MAX_AMPSWITCHS; i++) midiChannelMap[i] = i;
            nvs.end();
            nvs.begin("midi_map", false);
            nvs.putBytes("map", midiChannelMap, MAX_AMPSWITCHS);
            nvs.putInt("version", STORAGE_VERSION);
            nvs.end();
            log(LOG_WARN, "MIDI map NVS version mismatch, resetting to defaults");
        } else if (nvs.getBytesLength("map") == MAX_AMPSWITCHS) {
            nvs.getBytes("map", midiChannelMap, MAX_AMPSWITCHS);
            nvs.end();
            log(LOG_INFO, "MIDI channel map loaded from NVS");
        } else {
            nvs.end();
        }
    }
}

void handleProgramChange(byte midiChannel, byte program) {
    if (midiLearnChannel >= 0) {
        midiChannelMap[midiLearnChannel] = program;
        saveMidiMapToNVS();
        log(LOG_INFO, String("MIDI PC#") + String(program) + " assigned to channel " + String(midiLearnChannel+1));
        setStatusLedPattern(LED_SINGLE_FLASH);
        midiLearnChannel = -1;
        return;
    }
    // Normal operation: use mapping
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        if (midiChannelMap[i] == program) {
            setStatusLedPattern(LED_TRIPLE_FLASH);
            log(LOG_INFO, "MIDI Program Change - Channel: " + String(midiChannel) + ", Program: " + String(program) + " mapped to channel " + String(i+1));
            setAmpChannel(i + 1);
            return;
        }
    }
    log(LOG_INFO, "MIDI Program Change - Channel: " + String(midiChannel) + ", Program: " + String(program) + " (no mapping)");
}

void setAmpChannel(uint8_t channel) {
    if (channel == currentAmpChannel) {
        log(LOG_DEBUG, "Channel " + String(channel) + " already active, ignoring");
        return; // Already selected
    }

    log(LOG_INFO, "Switching amp channel from " + String(currentAmpChannel) + " to " + String(channel));

    // Turn all channels OFF
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        digitalWrite(ampSwitchPins[i], LOW);
    }

    // Turn ON the requested channel (if valid, 1-based index)
    if (channel >= 1 && channel <= MAX_AMPSWITCHS) {
        digitalWrite(ampSwitchPins[channel - 1], HIGH);
        currentAmpChannel = channel;
        log(LOG_INFO, "Amp channel " + String(channel) + " activated (pin " + String(ampSwitchPins[channel - 1]) + ")");
    } else if (channel == 0) {
        currentAmpChannel = 0;
        log(LOG_INFO, "All amp channels turned off");
    } else {
        currentAmpChannel = 0; // None selected
        log(LOG_WARN, "Invalid channel number: " + String(channel) + " (valid range: 0-" + String(MAX_AMPSWITCHS) + ")");
    }
    
    // Log current state
    log(LOG_DEBUG, "Current amp channel: " + String(currentAmpChannel));
}