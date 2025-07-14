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
    
    log(LOG_INFO, "=== ESP32 Client Starting ===");
    log(LOG_INFO, "Firmware Version: " + String(FIRMWARE_VERSION));
    log(LOG_INFO, "Board ID: " + String(BOARD_ID));
    
    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    log(LOG_DEBUG, "Status LED initialized on pin " + String(STATUS_LED_PIN));
    
    // Initialize pairing button
    pinMode(PAIRING_BUTTON_PIN, INPUT_PULLUP);
    log(LOG_DEBUG, "Pairing button initialized on pin " + String(PAIRING_BUTTON_PIN));
    
    // Setup pairing LED
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(PAIRING_LED_PIN, LEDC_CHANNEL_0);
    log(LOG_DEBUG, "Pairing LED initialized on pin " + String(PAIRING_LED_PIN));
    
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
    log(LOG_INFO, "Enter 'ota' within 10 seconds to enter OTA mode...");

    while (millis() - serialWaitStart < 10000) {
        checkSerialCommands();
        delay(10);
        if (serialOtaTrigger) break;
    }

    if (checkOtaTrigger() || serialOtaTrigger) {
        log(LOG_INFO, "OTA mode triggered, starting OTA...");
        startOTA();
        return;
    }

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
    
    // Check for button presses and serial commands
    checkPairingButton();
    checkAmpChannelButtons();
    updatePairingLED();
    checkSerialCommands();
    
    // Process MIDI messages
    MIDI.read();

    // Handle pairing if not paired
    if (pairingStatus != PAIR_PAIRED) {
        autoPairing();
        return; // Don't try to send data if not paired!
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
