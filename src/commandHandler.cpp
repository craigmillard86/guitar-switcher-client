#include "commandHandler.h"
#include "globals.h"
#include "espnow-pairing.h"
#include "utils.h"

void checkAmpChannelButtons() {
    static unsigned long lastDebounceTime[MAX_AMPSWITCHS] = {0};
    static uint8_t lastButtonState[MAX_AMPSWITCHS] = {HIGH};
    const unsigned long debounceDelay = 50; // ms

    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        uint8_t reading = digitalRead(ampButtonPins[i]);
        if (reading != lastButtonState[i]) {
            lastDebounceTime[i] = millis();
        }
        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
            if (reading == LOW && currentAmpChannel != (i + 1)) { // Button pressed
                setAmpChannel(i + 1);
            }
        }
        lastButtonState[i] = reading;
    }
}

void handleCommand(uint8_t commandType, uint8_t value) {
    switch (commandType) {
        case PROGRAM_CHANGE:
            setAmpChannel(value);
            log(LOG_ERROR, "[Client] Command: PROGRAM_CHANGE Value:" + String(value));
            break;
        default:
            log(LOG_WARN, "[Client] Unknown command received");
            break;
    }
}

void handleProgramChange(byte midiChannel, byte program) {
    setAmpChannel(program + 1); // MIDI PC#0 = channel 1, etc.
}

void setAmpChannel(uint8_t channel) {
    if (channel == currentAmpChannel) return; // Already selected

    // Turn all channels OFF
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        digitalWrite(ampSwitchPins[i], LOW);
    }

    // Turn ON the requested channel (if valid, 1-based index)
    if (channel >= 1 && channel <= MAX_AMPSWITCHS) {
        digitalWrite(ampSwitchPins[channel - 1], HIGH);
        currentChannel = channel;
    } else {
        currentAmpChannel = 0; // None selected
    }
}