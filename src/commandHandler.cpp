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

unsigned long midiLearnStartTime = 0;

// Shared variables for both modes
static unsigned long lastLedFlash = 0;
static bool ledFlashed = false;
static bool milestone5s = false;
static bool milestone10s = false;
static bool milestone15s = false;
static bool milestone20s = false;
static bool milestone25s = false;

// Channel select variables for both modes
static bool channelSelectMode = false;
static uint8_t buttonPressCount = 0;
static uint8_t tempMidiChannel = 1;
static unsigned long channelSelectStart = 0;

// Shared function to reset milestone flags
void resetMilestoneFlags() {
    milestone5s = false;
    milestone10s = false;
    milestone15s = false;
    milestone20s = false;
    milestone25s = false;
}

// Shared function to handle LED feedback
void handleLedFeedback(unsigned long held, const char* buttonName) {
    if (held >= 5000 && !milestone5s) {
        setStatusLedPattern(LED_SINGLE_FLASH);
        milestone5s = true;
        log(LOG_INFO, String(buttonName) + " - 5s held - LED feedback");
    } else if (held >= 10000 && !milestone10s) {
        setStatusLedPattern(LED_SINGLE_FLASH);
        milestone10s = true;
        log(LOG_INFO, String(buttonName) + " - 10s held - LED feedback");
    } else if (held >= 15000 && !milestone15s) {
        setStatusLedPattern(LED_SINGLE_FLASH);
        milestone15s = true;
        log(LOG_INFO, String(buttonName) + " - 15s held - Channel Select Mode Active!");
    } else if (held >= 20000 && !milestone20s) {
        setStatusLedPattern(LED_SINGLE_FLASH);
        milestone20s = true;
        log(LOG_INFO, String(buttonName) + " - 20s held - LED feedback");
    } else if (held >= 25000 && !milestone25s) {
        setStatusLedPattern(LED_SINGLE_FLASH);
        milestone25s = true;
        log(LOG_INFO, String(buttonName) + " - 25s held - LED feedback");
    }
}

// Shared function to handle channel select mode entry
void enterChannelSelectMode() {
    channelSelectMode = true;
    buttonPressCount = 0;
    tempMidiChannel = currentMidiChannel;
    channelSelectStart = millis();
    log(LOG_INFO, "15s long press: Channel Select Mode Active!");
    setStatusLedPattern(LED_FADE);
}

// Shared function to handle channel selection
void handleChannelSelection() {
    buttonPressCount++;
    tempMidiChannel = ((buttonPressCount - 1) % 16) + 1;
    log(LOG_INFO, "Button press " + String(buttonPressCount) + 
        " -> Channel " + String(tempMidiChannel));
    
    // Visual feedback
    for (int i = 0; i < tempMidiChannel; i++) {
        setStatusLedPattern(LED_SINGLE_FLASH);
        delay(200);
        setStatusLedPattern(LED_OFF);
        delay(100);
    }
}

// Shared function to handle auto-save
void handleChannelSelectAutoSave() {
    if (channelSelectMode && (millis() - channelSelectStart > 10000)) {
        currentMidiChannel = tempMidiChannel;
        saveMidiChannelToNVS();
        channelSelectMode = false;
        log(LOG_INFO, "Channel " + String(currentMidiChannel) + " selected and saved");
        
        // Flash LED the number of times corresponding to the selected channel
        for (int i = 0; i < currentMidiChannel; i++) {
            setStatusLedPattern(LED_SINGLE_FLASH);
            delay(200);
            setStatusLedPattern(LED_OFF);
            delay(100);
        }
    }
}

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

#if MAX_AMPSWITCHS > 1
    // MIDI Learn chord detection (multi-button only)
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

    // Block all button actions during MIDI Learn lockout
    if (midiLearnChannel >= 0) {
        // Timeout check
        if (millis() - midiLearnStartTime > MIDI_LEARN_TIMEOUT) {
            log(LOG_WARN, "MIDI Learn timed out, exiting learn mode.");
            midiLearnArmed = false;
            midiLearnChannel = -1;
        }
        return;
    }
#endif

#if MAX_AMPSWITCHS == 1
    // Single-button mode logic
    static unsigned long midiLearnArmedStart = 0;
    if (midiLearnArmed) {
        if (midiLearnArmedStart == 0) {
            midiLearnArmedStart = millis();
        }
        if (millis() - midiLearnArmedStart > MIDI_LEARN_TIMEOUT) {
            midiLearnArmed = false;
            midiLearnArmedStart = 0;
            log(LOG_WARN, "MIDI Learn timed out, exiting learn mode.");
            setStatusLedPattern(LED_OFF);
            buttonPressed[0] = false;
        }
        return;
    } else {
        midiLearnArmedStart = 0;
    }
    
    uint8_t reading = digitalRead(ampButtonPins[0]);
    static unsigned long buttonPressStart = 0;
    static bool buttonLongPressHandled = false;
    static bool midiLearnFirstLong = false;
    static unsigned long midiLearnWindowStart = 0;
    const unsigned long midiLearnWindow = 2000;

    if (reading != lastButtonState[0]) {
        lastDebounceTime[0] = millis();
    }
    if ((millis() - lastDebounceTime[0]) > debounceDelay) {
        if (reading == LOW && !buttonPressed[0]) {
            buttonPressStart = millis();
            buttonPressed[0] = true;
            buttonLongPressHandled = false;
            ledFlashed = false;
            lastLedFlash = 0;
            resetMilestoneFlags();
        } else if (reading == LOW && buttonPressed[0]) {
            unsigned long held = millis() - buttonPressStart;
            
            // Shared LED feedback
            handleLedFeedback(held, "Button 1");
            
            // Channel Select Mode (15s hold)
            if (!buttonLongPressHandled && held >= 15000) {
                enterChannelSelectMode();
                buttonLongPressHandled = true;
                midiLearnFirstLong = false;
            }
            // Pairing mode (30s hold)
            else if (!buttonLongPressHandled && held >= 30000) {
                clearPairingNVS();
                pairingStatus = NOT_PAIRED;
                log(LOG_INFO, "30s long press: Pairing mode triggered!");
                buttonLongPressHandled = true;
                midiLearnFirstLong = false;
                channelSelectMode = false;
            }
        } else if (reading == HIGH && buttonPressed[0]) {
            unsigned long held = millis() - buttonPressStart;
            
            if (!buttonLongPressHandled) {
                if (channelSelectMode) {
                    handleChannelSelection();
                }
                else if (midiLearnFirstLong) {
                    // This is the second long press for MIDI Learn
                    if (held >= 5000 && held < 10000) {
                        midiLearnArmed = true;
                        midiLearnChannel = 0;
                        log(LOG_INFO, "Double long press: MIDI Learn mode armed for single channel.");
                        setStatusLedPattern(LED_FAST_BLINK);
                    } else {
                        log(LOG_INFO, "Second long press too short/too long, MIDI Learn not armed.");
                    }
                    midiLearnFirstLong = false;
                } else if (held >= 5000 && held < 10000) {
                    // First long press for MIDI Learn, start window
                    midiLearnFirstLong = true;
                    midiLearnWindowStart = millis();
                    log(LOG_INFO, "First long press detected, waiting for second long press for MIDI Learn...");
                } else if (held < 5000) {
                    // Only toggle relay if NOT in channel select mode
                    if (!channelSelectMode) {
                        if (currentAmpChannel == 1) {
                            setAmpChannel(0);
                            log(LOG_INFO, "Toggled relay OFF");
                        } else {
                            setAmpChannel(1);
                            log(LOG_INFO, "Toggled relay ON");
                        }
                    }
                }
            }
            buttonPressed[0] = false;
            buttonLongPressHandled = false;
            resetMilestoneFlags();
        }
        
        // Shared auto-save logic
        handleChannelSelectAutoSave();
        
        // Cancel MIDI Learn window if time expires
        if (midiLearnFirstLong && !buttonPressed[0] && (millis() - midiLearnWindowStart > midiLearnWindow)) {
            midiLearnFirstLong = false;
            log(LOG_INFO, "MIDI Learn double long press window expired.");
        }
    }
    lastButtonState[0] = reading;

#else
    // Multi-button mode logic
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
                midiLearnStartTime = millis();
                log(LOG_INFO, String("MIDI Learn: Waiting for MIDI PC for channel ") + String(i+1));
                setStatusLedPattern(LED_TRIPLE_FLASH);
            }
            
            if (i == 0) { // Button 1: support long press for pairing
                if (reading == LOW && !buttonPressed[i]) {
                    button1PressStart = millis();
                    buttonPressed[i] = true;
                    button1LongPressHandled = false;
                    ledFlashed = false;
                    lastLedFlash = 0;
                    resetMilestoneFlags();
                } else if (reading == LOW && buttonPressed[i]) {
                    unsigned long held = millis() - button1PressStart;
                    
                    // Shared LED feedback
                    handleLedFeedback(held, "Button 1");
                    
                    // Channel Select Mode (15s hold)
                    if (!button1LongPressHandled && held >= 15000) {
                        enterChannelSelectMode();
                        button1LongPressHandled = true;
                    }
                    // Pairing mode (30s hold)
                    else if (!button1LongPressHandled && held >= 30000) {
                        clearPairingNVS();
                        pairingStatus = NOT_PAIRED;
                        log(LOG_INFO, "30s long press: Pairing mode triggered!");
                        button1LongPressHandled = true;
                        channelSelectMode = false;
                    }
                } else if (reading == HIGH && buttonPressed[i]) {
                    // Released
                    if (!button1LongPressHandled) {
                        if (channelSelectMode) {
                            handleChannelSelection();
                        } else if (millis() - button1PressStart < longPressTime) {
                            if (!midiLearnArmed) {
                                log(LOG_INFO, "Button 1 short press: switching to channel 1");
                                setAmpChannel(1);
                            }
                        }
                    }
                    buttonPressed[i] = false;
                    button1LongPressHandled = false;
                    resetMilestoneFlags();
                }
            } else {
                // All other buttons: trigger on release (no LED feedback)
                if (reading == LOW && !buttonPressed[i]) {
                    buttonPressed[i] = true;
                } else if (reading == HIGH && buttonPressed[i]) {
                    if (!midiLearnArmed) {
                        log(LOG_INFO, "Button " + String(i + 1) + " released, switching to channel " + String(i + 1));
                        setAmpChannel(i + 1);
                    }
                    buttonPressed[i] = false;
                }
            }
        }
        lastButtonState[i] = reading;
    }
    
    // Shared auto-save logic
    handleChannelSelectAutoSave();
#endif
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

void saveMidiChannelToNVS() {
    Preferences nvs;
    if (nvs.begin("midi_channel", false)) {
        nvs.putUChar("channel", currentMidiChannel);
        nvs.putInt("version", STORAGE_VERSION);
        nvs.end();
        log(LOG_INFO, "MIDI channel " + String(currentMidiChannel) + " saved to NVS");
    } else {
        log(LOG_ERROR, "Failed to save MIDI channel to NVS");
    }
}

void loadMidiChannelFromNVS() {
    Preferences nvs;
    if (nvs.begin("midi_channel", true)) {
        if (nvs.getInt("version", 0) != STORAGE_VERSION) {
            // Version mismatch: reset to defaults and save
            currentMidiChannel = 1;
            nvs.end();
            nvs.begin("midi_channel", false);
            nvs.putUChar("channel", currentMidiChannel);
            nvs.putInt("version", STORAGE_VERSION);
            nvs.end();
            log(LOG_WARN, "MIDI channel NVS version mismatch, resetting to default");
        } else if (nvs.isKey("channel")) {
            currentMidiChannel = nvs.getUChar("channel", 1);
            nvs.end();
            log(LOG_INFO, "MIDI channel " + String(currentMidiChannel) + " loaded from NVS");
        } else {
            nvs.end();
        }
    }
}

void handleProgramChange(byte midiChannel, byte program) {
    if (midiChannel != currentMidiChannel) return; // Only respond to selected channel

#if MAX_AMPSWITCHS == 1
    // MIDI Learn mode (if you want to support mapping, otherwise skip this block)
    if (midiLearnChannel >= 0 || midiLearnArmed) {
        midiChannelMap[0] = program;
        saveMidiMapToNVS();
        log(LOG_INFO, String("MIDI PC#") + String(program) + " assigned to channel 1");
        setStatusLedPattern(LED_SINGLE_FLASH);
        midiLearnChannel = -1;
        midiLearnArmed = false;
        return;
    }
    // Normal operation: toggle relay on any PC message
    if (program == midiChannelMap[0]) {
        if (currentAmpChannel == 1) {
            setAmpChannel(0);
            log(LOG_INFO, "MIDI PC received: Toggled relay OFF");
        } else {
            setAmpChannel(1);
            log(LOG_INFO, "MIDI PC received: Toggled relay ON");
        }
        setStatusLedPattern(LED_TRIPLE_FLASH);
    }
    return;
#else
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
#endif
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