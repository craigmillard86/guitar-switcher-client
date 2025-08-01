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
#include "pairing.h"
#include "espnow.h"
#include "globals.h"
#include "utils.h"
#include "espnow-pairing.h"
#include "commandHandler.h"
#include <esp_now.h>
#include <WiFi.h>
#include <espnow-pairing.h>

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        log(LOG_DEBUG, "Data sent successfully to ");
        printMAC(mac_addr, LOG_DEBUG);
    } else {
        log(LOG_WARN, "Data send failed to ");
        printMAC(mac_addr, LOG_WARN);
    }
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
    uint8_t type = incomingData[0];
    
    if (pairingStatus != PAIR_PAIRED && type != PAIRING) {
        log(LOG_DEBUG, "Ignoring data: not paired");
        return;
    }
    
    log(LOG_DEBUG, "Packet received from ");
    printMAC(mac_addr, LOG_DEBUG);
    logf(LOG_DEBUG, "Data size: %u bytes", len);
    
    switch (type) {
        case DATA:      // we received data from server
            setStatusLedPattern(LED_SINGLE_FLASH);
            memcpy(&inData, incomingData, sizeof(inData));
            log(LOG_DEBUG, "Data packet received:");
            logf(LOG_DEBUG, "  ID: %lu", inData.id);
            logf(LOG_DEBUG, "  Temperature: %.1f", inData.temp);
            logf(LOG_DEBUG, "  Humidity: %.1f", inData.hum);
            logf(LOG_DEBUG, "  Reading ID: %lu", inData.readingId);

            if (inData.readingId % 2 == 1) {
                digitalWrite(LED_BUILTIN, LOW);
            } else { 
                digitalWrite(LED_BUILTIN, HIGH);
            }
            break;

        case PAIRING:    // we received pairing data from server
            setStatusLedPattern(LED_SINGLE_FLASH);
            memcpy(&pairingData, incomingData, sizeof(pairingData));
            if (pairingData.id == 0) {              // the message comes from server
                log(LOG_INFO, "Pairing successful!");
                log(LOG_INFO, "Server MAC Address: ");
                printMAC(pairingData.macAddr, LOG_INFO);
                logf(LOG_INFO, "Channel: %u", pairingData.channel);
                
                log(LOG_DEBUG, "Adding peer to ESP-NOW...");
                addPeer(pairingData.macAddr, pairingData.channel); // add the server to the peer list 
                log(LOG_DEBUG, "Peer added successfully");
                
                log(LOG_DEBUG, "Setting pairing status to PAIR_PAIRED");
                pairingStatus = PAIR_PAIRED;             // set the pairing status
                log(LOG_INFO, "Pairing process completed successfully");
            }
            break;

        case COMMAND:
            setStatusLedPattern(LED_DOUBLE_FLASH);
            log(LOG_INFO, "Command received from server");
            handleCommand(inData.commandType, inData.commandValue);
            break;
            
        default:
            logf(LOG_WARN, "Unknown message type: %u", type);
            break;
    }  
}

void initESP_NOW(){
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        log(LOG_ERROR, "Error initializing ESP-NOW");
        return;
    }
    
    log(LOG_DEBUG, "ESP-NOW initialized successfully");
    
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
    
    log(LOG_DEBUG, "ESP-NOW callbacks registered");
} 