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

    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        uint8_t reading = digitalRead(ampButtonPins[i]);
        
        // Only process if button state changed
        if (reading != lastButtonState[i]) {
            lastDebounceTime[i] = millis();
        }
        
        // Check if enough time has passed since last change
        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
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

void handleProgramChange(byte midiChannel, byte program) {
    setStatusLedPattern(LED_TRIPLE_FLASH);
    log(LOG_INFO, "MIDI Program Change - Channel: " + String(midiChannel) + ", Program: " + String(program));
    setAmpChannel(program + 1); // MIDI PC#0 = channel 1, etc.
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