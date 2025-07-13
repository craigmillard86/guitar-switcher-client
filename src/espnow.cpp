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
    log(LOG_DEBUG, "Data size: " + String(len) + " bytes");
    
    switch (type) {
        case DATA:      // we received data from server
            memcpy(&inData, incomingData, sizeof(inData));
            log(LOG_DEBUG, "Data packet received:");
            log(LOG_DEBUG, "  ID: " + String(inData.id));
            log(LOG_DEBUG, "  Temperature: " + String(inData.temp));
            log(LOG_DEBUG, "  Humidity: " + String(inData.hum));
            log(LOG_DEBUG, "  Reading ID: " + String(inData.readingId));

            if (inData.readingId % 2 == 1) {
                digitalWrite(LED_BUILTIN, LOW);
            } else { 
                digitalWrite(LED_BUILTIN, HIGH);
            }
            break;

        case PAIRING:    // we received pairing data from server
            memcpy(&pairingData, incomingData, sizeof(pairingData));
            if (pairingData.id == 0) {              // the message comes from server
                log(LOG_INFO, "Pairing successful!");
                log(LOG_INFO, "Server MAC Address: ");
                printMAC(pairingData.macAddr, LOG_INFO);
                log(LOG_INFO, "Channel: " + String(pairingData.channel));
                
                log(LOG_DEBUG, "Adding peer to ESP-NOW...");
                addPeer(pairingData.macAddr, pairingData.channel); // add the server to the peer list 
                log(LOG_DEBUG, "Peer added successfully");
                
                log(LOG_DEBUG, "Setting pairing status to PAIR_PAIRED");
                pairingStatus = PAIR_PAIRED;             // set the pairing status
                log(LOG_INFO, "Pairing process completed successfully");
            }
            break;

        case COMMAND:
            log(LOG_INFO, "Command received from server");
            handleCommand(inData.commandType, inData.commandValue);
            break;
            
        default:
            log(LOG_WARN, "Unknown message type: " + String(type));
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