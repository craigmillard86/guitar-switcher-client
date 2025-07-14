#include <Arduino.h>
#include "config.h"
#include "utils.h"
#include <cstring>

// Global configuration variables
ClientType currentClientType = CLIENT_TYPE_ENUM;

// Pin arrays are defined in globals.cpp
// These are just references to the existing arrays

// Parse pin array from string (e.g., "4,5,6,7")
uint8_t* parsePinArray(const char* pinString) {
    static uint8_t pins[16]; // Max 16 pins
    int pinCount = 0;
    
    char* str = strdup(pinString);
    char* token = strtok(str, ",");
    
    while (token != NULL && pinCount < 16) {
        pins[pinCount++] = atoi(token);
        token = strtok(NULL, ",");
    }
    
    free(str);
    return pins;
}

String getClientTypeString() {
    switch (currentClientType) {
        case CLIENT_AMP_SWITCHER:
            return "AMP_SWITCHER";
        case CLIENT_CUSTOM:
            return "CUSTOM";
        default:
            return "UNKNOWN";
    }
}

void printClientConfiguration() {
    log(LOG_INFO, "=== CLIENT CONFIGURATION ===");
    log(LOG_INFO, "Client Type: " + getClientTypeString());
    log(LOG_INFO, String("Device Name: ") + DEVICE_NAME);
    
    #if HAS_AMP_SWITCHING
    log(LOG_INFO, "Amp Switching: Enabled");
    log(LOG_INFO, "Max Amp Switches: " + String(MAX_AMPSWITCHS));
    log(LOG_INFO, String("Amp Switch Pins: ") + AMP_SWITCH_PINS);
    log(LOG_INFO, String("Amp Button Pins: ") + AMP_BUTTON_PINS);
    #else
    log(LOG_INFO, "Amp Switching: Disabled");
    #endif
    
    log(LOG_INFO, "==========================");
}

void initializeClientConfiguration() {
    log(LOG_INFO, "Initializing client configuration...");
    
    #if HAS_AMP_SWITCHING
    // Parse and set amp switch pins
    uint8_t* switchPins = parsePinArray(AMP_SWITCH_PINS);
    uint8_t* buttonPins = parsePinArray(AMP_BUTTON_PINS);
    
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        ampSwitchPins[i] = switchPins[i];
        ampButtonPins[i] = buttonPins[i];
        pinMode(ampButtonPins[i], INPUT_PULLUP);
        pinMode(ampSwitchPins[i], OUTPUT);
        digitalWrite(ampSwitchPins[i], LOW); // Ensure relays are off at boot
    }
    digitalWrite(ampSwitchPins[0], HIGH);
    log(LOG_DEBUG, "Amp switching pins initialized");
    #endif
    
    printClientConfiguration();
} 