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
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <esp_pm.h>
#include <esp_wifi_types.h>
#include <dataStructs.h>
#include "config.h"
#include "pairing.h"
#include "espnow-pairing.h"
#include <globals.h>
#include <utils.h>
#include <otaManager.h>
#include <espnow.h>
#include <MIDI.h>
#include <commandHandler.h>
#include "nvsManager.h"
#include "debug.h"

MessageType messageType;
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup() {
    delay(5000);
    Serial.begin(115200);
    
    // Initialize performance metrics
    perfMetrics.startTime = millis();
    
    // Load log level from NVS
    currentLogLevel = loadLogLevelFromNVS();
    
    // Initialize client configuration
    initializeClientConfiguration();

    // Load MIDI mapping from NVS
    loadMidiMapFromNVS();
    loadMidiChannelFromNVS();
    
    log(LOG_INFO, "=== ESP32 Client Starting ===");
    log(LOG_INFO, "Firmware Version: " + String(FIRMWARE_VERSION));
    log(LOG_INFO, "Board ID: " + String(BOARD_ID));
    
     
    // Setup pairing LED
    pinMode(PAIRING_LED_PIN, OUTPUT);
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(PAIRING_LED_PIN, LEDC_CHANNEL_0);
    log(LOG_DEBUG, "Pairing LED initialized on pin " + String(PAIRING_LED_PIN));
    setStatusLedPattern(LED_OFF);
    
    log(LOG_INFO, "Hardware initialization complete");
    
    // Initialize WiFi
    log(LOG_DEBUG, "Initializing WiFi...");
    WiFi.mode(WIFI_STA);
    esp_wifi_set_ps(WIFI_PS_NONE);
    log(LOG_DEBUG, "WiFi initialized in station mode");
    
    // Get and display MAC address
    esp_wifi_get_mac(WIFI_IF_STA, clientMacAddress);
    log(LOG_INFO, "Client Board MAC Address: ");
    printMAC(clientMacAddress, LOG_INFO);
    
    WiFi.disconnect();
    start = millis();
    
    // OTA trigger check
    unsigned long serialWaitStart = millis();
    log(LOG_INFO, "Enter 'ota' within 10 seconds or hold Button 1 for 5s to enter OTA mode...");

    bool button1OtaTriggered = false;
    const unsigned long otaButtonHoldTime = 5000; // 5 seconds
    bool button1WasPressed = false;
    unsigned long button1PressStart = 0;

    while (millis() - serialWaitStart < 10000) {
        checkSerialCommands();
        delay(10);
        if (serialOtaTrigger) break;

        // Check for long press on button 1 (ampButtonPins[0])
        int button1State = digitalRead(ampButtonPins[0]);
        if (button1State == LOW && !button1WasPressed) {
            button1PressStart = millis();
            button1WasPressed = true;
        } else if (button1State == LOW && button1WasPressed) {
            if (!button1OtaTriggered && (millis() - button1PressStart > otaButtonHoldTime)) {
                serialOtaTrigger = true;
                button1OtaTriggered = true;
                log(LOG_INFO, "OTA mode triggered by holding Button 1");
                break;
            }
        } else if (button1State == HIGH && button1WasPressed) {
            button1WasPressed = false;
            button1PressStart = 0;
        }
    }

    if (serialOtaTrigger) {
        log(LOG_INFO, "OTA mode triggered, starting OTA...");
        updateStatusLED();
        startOTA_AP();
        return;
    }
    serialOtaTrigger = false; // Prevent re-entry
    // Check for existing pairing
    if (!loadServerFromNVS(serverAddress, &currentChannel)) {
        log(LOG_WARN, "No paired server found in NVS, starting pairing...");
        pairingStatus = PAIR_REQUEST;
        autoPairing();
    } else {
        pairingStatus = PAIR_PAIRED;
        log(LOG_INFO, "Loaded paired server from NVS:");
        printMAC(serverAddress, LOG_INFO);
        log(LOG_INFO, "Channel: " + String(currentChannel));
        
        // Set the WiFi channel
        ESP_ERROR_CHECK(esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE));
        log(LOG_DEBUG, "WiFi channel set to " + String(currentChannel));

        // Initialize ESP-NOW
        log(LOG_DEBUG, "Initializing ESP-NOW...");
        initESP_NOW();

        // Add peer to ESP-NOW
        addPeer(serverAddress, currentChannel);
        log(LOG_DEBUG, "Peer added to ESP-NOW");
    }

    // Initialize MIDI
    log(LOG_DEBUG, "Initializing MIDI...");
    Serial1.begin(31250, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
    MIDI.setHandleProgramChange(handleProgramChange);
    MIDI.begin(MIDI_CHANNEL_OMNI); // Listen to all channels
    MIDI.turnThruOn();             // Pass incoming MIDI to output (MIDI THRU)
    log(LOG_INFO, "MIDI initialized on pins RX:" + String(MIDI_RX_PIN) + " TX:" + String(MIDI_TX_PIN));

    log(LOG_INFO, "=== Setup Complete ===");
    log(LOG_INFO, "Type 'help' for available commands");
    log(LOG_INFO, "Type 'status' for system information");
}

void loop() {
    unsigned long loopStart = millis();
    
    // Always check for button presses and serial commands (regardless of pairing status)
    checkAmpChannelButtons();
    updateStatusLED(); // Replaces updatePairingLED()
    checkSerialCommands();

    // Add this block at the top of loop()
    if (serialOtaTrigger) {
        log(LOG_INFO, "OTA mode triggered, starting OTA...");
        updateStatusLED(); // Replaces setStatusLedPattern(LED_FAST_BLINK);
        startOTA_AP();
        serialOtaTrigger = false; // Prevent re-entry
        ESP.restart(); // Optional: reboot after OTA
        return;
    }

    // Process MIDI messages
    MIDI.read();

    // Handle pairing if not paired (but don't return early)
    if (pairingStatus != PAIR_PAIRED) {
        autoPairing();
        // Don't return early - continue with the rest of the loop
    }
    
    // Update performance metrics
    unsigned long loopTime = millis() - loopStart;
    updatePerformanceMetrics(loopTime);
    
    // Periodic memory check (every 30 seconds)
    static unsigned long lastMemoryCheck = 0;
    if (millis() - lastMemoryCheck > 30000) {
        lastMemoryCheck = millis();
        updateMemoryStats();
        uint32_t freeHeap = getFreeHeap();
        if (freeHeap < 10000) { // Warning if less than 10KB free
            log(LOG_WARN, "Low memory warning: " + String(freeHeap) + "B free");
        }
    }
}
