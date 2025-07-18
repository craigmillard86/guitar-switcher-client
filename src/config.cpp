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
//
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
    log(LOG_INFO, String("Client Type: ") + getClientTypeString());
    log(LOG_INFO, String("Device Name: ") + deviceName);
    
#if HAS_AMP_SWITCHING
    log(LOG_INFO, "Amp Switching: Enabled");
    log(LOG_INFO, "Max Amp Switches: " + String(MAX_AMPSWITCHS));
    // Print ampSwitchPins as comma-separated string
    String switchPinsStr;
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        switchPinsStr += String(ampSwitchPins[i]);
        if (i < MAX_AMPSWITCHS - 1) switchPinsStr += ",";
    }
    log(LOG_INFO, "Amp Switch Pins: " + switchPinsStr);
    // Print ampButtonPins as comma-separated string
    String buttonPinsStr;
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        buttonPinsStr += String(ampButtonPins[i]);
        if (i < MAX_AMPSWITCHS - 1) buttonPinsStr += ",";
    }
    log(LOG_INFO, "Amp Button Pins: " + buttonPinsStr);
#else
    log(LOG_INFO, "Amp Switching: Disabled");
#endif
    log(LOG_INFO, "==========================");
}

void initializeClientConfiguration() {
    log(LOG_INFO, "Initializing client configuration...");
    
#if HAS_AMP_SWITCHING
    // Parse and set amp switch pins from macro
    uint8_t* switchPins = parsePinArray(AMP_SWITCH_PINS);
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        ampSwitchPins[i] = switchPins[i];
    }
    uint8_t* buttonPins = parsePinArray(AMP_BUTTON_PINS);
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        ampButtonPins[i] = buttonPins[i];
        pinMode(ampButtonPins[i], INPUT_PULLUP);
        pinMode(ampSwitchPins[i], OUTPUT);
        digitalWrite(ampSwitchPins[i], LOW); // Ensure relays are off at boot
    }
    digitalWrite(ampSwitchPins[0], HIGH);
    log(LOG_DEBUG, "Amp switching pins initialized");
#endif
    // Set device name from macro
    strncpy(deviceName, DEVICE_NAME, MAX_PEER_NAME_LEN-1);
    deviceName[MAX_PEER_NAME_LEN-1] = '\0';
    printClientConfiguration();
} 