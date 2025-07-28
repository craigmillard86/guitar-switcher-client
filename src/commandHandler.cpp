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
#include "nvsManager.h"

unsigned long midiLearnStartTime = 0;
static bool midiLearnJustTimedOut = false; // Flag to prevent pairing mode after MIDI Learn timeout
static unsigned long midiLearnCompleteTime = 0; // Time when MIDI Learn completed
static const unsigned long MIDI_LEARN_COOLDOWN = 2000; // 2 second cooldown after MIDI Learn

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
static unsigned long lastChannelButtonPress = 0;

// Channel confirmation variables for non-blocking LED feedback
static bool showingChannelConfirmation = false;
static uint8_t confirmationChannel = 0;
static uint8_t confirmationFlashCount = 0;
static unsigned long lastConfirmationFlash = 0;
static bool confirmationLedState = false;

// Function to start non-blocking channel confirmation
void showChannelConfirmation(uint8_t channel) {
    showingChannelConfirmation = true;
    confirmationChannel = channel;
    confirmationFlashCount = 0;
    lastConfirmationFlash = millis();
    confirmationLedState = false;
    setStatusLedPattern(LED_OFF); // Start with LED off
}

// Function to handle non-blocking channel confirmation LED flashing
void updateChannelConfirmation() {
    if (!showingChannelConfirmation) return;
    
    unsigned long now = millis();
    
    if (confirmationFlashCount < confirmationChannel * 2) { // *2 because we flash on and off
        if (now - lastConfirmationFlash >= 200) { // 200ms intervals
            confirmationLedState = !confirmationLedState;
            if (confirmationLedState) {
                setStatusLedPattern(LED_SINGLE_FLASH);
            } else {
                setStatusLedPattern(LED_OFF);
            }
            confirmationFlashCount++;
            lastConfirmationFlash = now;
        }
    } else {
        // Finished flashing, return to normal operation
        showingChannelConfirmation = false;
        setStatusLedPattern(LED_OFF); // Return to normal operation (LED off)
    }
}

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
        log(LOG_INFO, String(buttonName) + " - 10s held - MIDI Learn ready");
    } else if (held >= 15000 && !milestone15s) {
        setStatusLedPattern(LED_SINGLE_FLASH);
        milestone15s = true;
        log(LOG_INFO, String(buttonName) + " - 15s held - Channel Select ready");
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
    lastChannelButtonPress = millis(); // Initialize the last button press time
    log(LOG_INFO, "15s long press: Channel Select Mode Active!");
    setStatusLedPattern(LED_FADE);
}

// Shared function to handle channel selection
void handleChannelSelection() {
    buttonPressCount++;
    tempMidiChannel = ((buttonPressCount - 1) % 16) + 1;
    lastChannelButtonPress = millis(); // Update the last button press time
    log(LOG_INFO, "Button press " + String(buttonPressCount) + 
        " -> Channel " + String(tempMidiChannel));
    
    // Immediate visual feedback - single flash to acknowledge button press
    setStatusLedPattern(LED_SINGLE_FLASH);
}

// Shared function to handle auto-save
void handleChannelSelectAutoSave() {
    if (channelSelectMode && (millis() - lastChannelButtonPress > 5000)) {
        currentMidiChannel = tempMidiChannel;
        saveMidiChannelToNVS();
        channelSelectMode = false;
        log(LOG_INFO, "Channel " + String(currentMidiChannel) + " selected and saved");
        
        // Non-blocking LED feedback showing the selected channel number
        // This will be handled by a separate function to avoid blocking
        showChannelConfirmation(currentMidiChannel);
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
    static unsigned long buttonPressStart[MAX_AMPSWITCHS] = {0};
    static bool buttonLongPressHandled[MAX_AMPSWITCHS] = {false};
    const unsigned long debounceDelay = BUTTON_DEBOUNCE_MS;
    const unsigned long longPressTime = BUTTON_LONGPRESS_MS;
    const unsigned long midiLearnActivationTime = 10000; // 10 seconds for MIDI Learn

    // Block all button actions during MIDI Learn lockout
    if (midiLearnChannel >= 0) {
        // Timeout check
        if (millis() - midiLearnStartTime > MIDI_LEARN_TIMEOUT) {
            log(LOG_WARN, "MIDI Learn timed out, exiting learn mode.");
            midiLearnArmed = false;
            midiLearnChannel = -1;
            midiLearnJustTimedOut = true; // Set flag to prevent pairing mode trigger
            setStatusLedPattern(LED_OFF);
            
            // Reset button states to prevent triggering pairing mode on release
            for (int i = 0; i < MAX_AMPSWITCHS; i++) {
                buttonLongPressHandled[i] = true; // Mark as handled to prevent further actions
            }
        }
        return;
    }

    // Unified button processing for all buttons
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        uint8_t reading = digitalRead(ampButtonPins[i]);
        
        // Debounce check
        if (reading != lastButtonState[i]) {
            lastDebounceTime[i] = millis();
        }
        
        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
            // Button pressed
            if (reading == LOW && !buttonPressed[i]) {
                buttonPressStart[i] = millis();
                buttonPressed[i] = true;
                buttonLongPressHandled[i] = false;
                if (i == 0) { // Only button 1 gets LED feedback
                    ledFlashed = false;
                    lastLedFlash = 0;
                    resetMilestoneFlags();
                }
                
                // Multi-button MIDI Learn channel selection
                #if MAX_AMPSWITCHS > 1
                if (midiLearnArmed) {
                    midiLearnChannel = i;
                    midiLearnArmed = false;
                    midiLearnStartTime = millis();
                    log(LOG_INFO, String("MIDI Learn: Waiting for MIDI PC for channel ") + String(i+1));
                    setStatusLedPattern(LED_TRIPLE_FLASH);
                }
                #endif
            }
            // Button held
            else if (reading == LOW && buttonPressed[i]) {
                unsigned long held = millis() - buttonPressStart[i];
                
                // Only button 1 supports long press functions
                if (i == 0) {
                    // LED feedback for button 1
                    handleLedFeedback(held, "Button 1");
                }
            }
            // Button released
            else if (reading == HIGH && buttonPressed[i]) {
                unsigned long held = millis() - buttonPressStart[i];
                
                if (!buttonLongPressHandled[i]) {
                    if (channelSelectMode) {
                        // In channel select mode, any button can be used for selection
                        handleChannelSelection();
                    } else if (held >= 30000 && !midiLearnJustTimedOut && i == 0) {
                        // Pairing mode (30s+ hold released) - only button 1, not if MIDI Learn just timed out
                        clearPairingNVS();
                        pairingStatus = NOT_PAIRED;
                        log(LOG_INFO, "30s+ hold released: Pairing mode triggered!");
                        channelSelectMode = false;
                    } else if (held >= 15000 && i == 0) {
                        // Channel Select Mode (15s+ hold released) - only button 1
                        enterChannelSelectMode();
                    } else if (held >= midiLearnActivationTime && i == 0) {
                        // MIDI Learn mode (10s+ hold released) - only button 1, unified for both single and multi-button
                        midiLearnArmed = true;
                        #if MAX_AMPSWITCHS == 1
                        midiLearnChannel = 0; // Single channel device - start learning immediately
                        midiLearnStartTime = millis();
                        log(LOG_INFO, "10s+ hold released: MIDI Learn mode armed for single channel.");
                        #else
                        log(LOG_INFO, "10s+ hold released: MIDI Learn mode armed. Press a channel button to select.");
                        #endif
                        setStatusLedPattern(LED_FAST_BLINK);
                    } else if (held < longPressTime) {
                        // Short press handling
                        // Check cooldown period after MIDI Learn completion
                        if (midiLearnCompleteTime > 0 && (millis() - midiLearnCompleteTime < MIDI_LEARN_COOLDOWN)) {
                            log(LOG_DEBUG, "Button press ignored during post-learn cooldown period");
                        } else if (!midiLearnArmed) {
                            #if MAX_AMPSWITCHS == 1
                            // Single button: toggle relay
                            if (currentAmpChannel == 1) {
                                setAmpChannel(0);
                                log(LOG_INFO, "Toggled relay OFF");
                            } else {
                                setAmpChannel(1);
                                log(LOG_INFO, "Toggled relay ON");
                            }
                            #else
                            // Multi-button: switch to specific channel
                            log(LOG_INFO, "Button " + String(i + 1) + " short press: switching to channel " + String(i + 1));
                            setAmpChannel(i + 1);
                            #endif
                        }
                    }
                }
                
                buttonPressed[i] = false;
                buttonLongPressHandled[i] = false;
                midiLearnJustTimedOut = false; // Reset flag when any button is released
                if (i == 0) {
                    resetMilestoneFlags();
                }
            }
        }
        lastButtonState[i] = reading;
    }
    
    // Update non-blocking channel confirmation LED feedback
    updateChannelConfirmation();
    
    // Shared auto-save logic
    handleChannelSelectAutoSave();
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
    if (midiChannel != currentMidiChannel) return; // Only respond to selected channel

#if MAX_AMPSWITCHS == 1
    // MIDI Learn mode - standardized timeout handling
    if (midiLearnChannel >= 0) {
        // Check timeout
        if (millis() - midiLearnStartTime > MIDI_LEARN_TIMEOUT) {
            log(LOG_WARN, "MIDI Learn timed out, exiting learn mode.");
            midiLearnArmed = false;
            midiLearnChannel = -1;
            setStatusLedPattern(LED_OFF);
            return;
        }
        
        // Learn the MIDI PC mapping
        midiChannelMap[0] = program;
        saveMidiMapToNVS();
        log(LOG_INFO, String("MIDI PC#") + String(program) + " assigned to channel 1");
        setStatusLedPattern(LED_SINGLE_FLASH);
        midiLearnChannel = -1;
        midiLearnArmed = false;
        midiLearnCompleteTime = millis(); // Set cooldown time
        return;
    }
    
    // Normal operation: toggle relay when mapped PC is received
    // Check cooldown period after MIDI Learn completion
    if (midiLearnCompleteTime > 0 && (millis() - midiLearnCompleteTime < MIDI_LEARN_COOLDOWN)) {
        log(LOG_DEBUG, "MIDI PC ignored during post-learn cooldown period");
        return;
    }
    
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
    // Multi-button mode - MIDI Learn with timeout handling
    if (midiLearnChannel >= 0) {
        // Check timeout
        if (millis() - midiLearnStartTime > MIDI_LEARN_TIMEOUT) {
            log(LOG_WARN, "MIDI Learn timed out, exiting learn mode.");
            midiLearnArmed = false;
            midiLearnChannel = -1;
            return;
        }
        
        // Learn the MIDI PC mapping
        midiChannelMap[midiLearnChannel] = program;
        saveMidiMapToNVS();
        log(LOG_INFO, String("MIDI PC#") + String(program) + " assigned to channel " + String(midiLearnChannel + 1));
        setStatusLedPattern(LED_SINGLE_FLASH);
        midiLearnChannel = -1;
        midiLearnCompleteTime = millis(); // Set cooldown time
        return;
    }
    
    // Normal operation: use mapping
    // Check cooldown period after MIDI Learn completion
    if (midiLearnCompleteTime > 0 && (millis() - midiLearnCompleteTime < MIDI_LEARN_COOLDOWN)) {
        log(LOG_DEBUG, "MIDI PC ignored during post-learn cooldown period");
        return;
    }
    
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