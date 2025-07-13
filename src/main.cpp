#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <esp_pm.h>
#include <esp_wifi_types.h>
#include <dataStructs.h>
#include <globals.h>
#include <utils.h>
#include <espnow-pairing.h>
#include <otaManager.h>
#include <espnow.h>
#include <MIDI.h>
#include <commandHandler.h>

MessageType messageType;
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup() {
    delay(5000);
    Serial.begin(115200);
    for (int i = 0; i < MAX_AMPSWITCHS; i++) {
        pinMode(ampSwitchPins[i], OUTPUT);
        digitalWrite(ampSwitchPins[i], LOW); // Start all OFF
    }
    digitalWrite(ampSwitchPins[0], HIGH);
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(PAIRING_BUTTON_PIN, INPUT_PULLUP);
      //Setup LED
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(PAIRING_LED_PIN, LEDC_CHANNEL_0);
    log(LOG_ERROR, "Client setup starting...");
    WiFi.mode(WIFI_STA);
    esp_wifi_set_ps(WIFI_PS_NONE);
    //WiFi.begin();
    log(LOG_ERROR,"Client Board MAC Address:  ");
    esp_wifi_get_mac(WIFI_IF_STA, clientMacAddress);
    printMAC(clientMacAddress, LOG_INFO);
    WiFi.disconnect();
    start = millis();
    unsigned long serialWaitStart = millis();
    log(LOG_ERROR,"Enter 'ota' within 10 seconds to enter OTA mode...");

    while (millis() - serialWaitStart < 10000) {
        checkSerialCommands();
        delay(10);
        if (serialOtaTrigger) break;
    }

    if (checkOtaTrigger() || serialOtaTrigger) {
        //updatePairingLED();
        startOTA();
        return;
    }
        printMAC(clientMacAddress, LOG_INFO);

    if (!loadServerFromNVS(serverAddress, &currentChannel)) {
        log(LOG_ERROR, "[Client] No paired server in NVS, starting pairing...");
        pairingStatus = PAIR_REQUEST;
        autoPairing();
    } else {
        pairingStatus = PAIR_PAIRED;
        log(LOG_ERROR, "[Client] Loaded paired server from NVS:");
        printMAC(serverAddress, LOG_INFO);
         // Set the WiFi channel
        ESP_ERROR_CHECK(esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE));

        initESP_NOW();

        // Add peer to ESP-NOW
        addPeer(serverAddress, currentChannel);
    }

    log(LOG_ERROR, "Client setup complete!");
    Serial1.begin(31250, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
    MIDI.setHandleProgramChange(handleProgramChange);
    MIDI.begin(MIDI_CHANNEL_OMNI); // Listen to all channels
    MIDI.turnThruOn();             // Pass incoming MIDI to output (MIDI THRU)
}

void loop() {
checkPairingButton();
checkAmpChannelButtons();
updatePairingLED();
checkSerialCommands();
MIDI.read();

if (pairingStatus != PAIR_PAIRED) {
    autoPairing();
    return; // Don't try to send data if not paired!
}
}
