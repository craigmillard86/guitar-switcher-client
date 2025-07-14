#include <Arduino.h>
#include "config.h"
#include "pairing.h"
#include "espnow-pairing.h"
#include "globals.h"
#include "utils.h"
#include <Preferences.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <espnow.h>

unsigned long currentMillis = millis();
unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings
unsigned long start;                // used to measure Pairing time
unsigned int readingId = 0;   
Preferences nvs;

void updatePairingLED() {
  static uint16_t fadeValue = 0;
  static int8_t fadeDirection = 1;

  if (pairingStatus == PAIR_REQUEST) {
    // Fade effect during Pairing mode only
    fadeValue += fadeDirection * 50;
    if (fadeValue >= 8191) {
      fadeValue = 8191;
      fadeDirection = -1;
    } else if (fadeValue <= 0) {
      fadeValue = 0;
      fadeDirection = 1;
    }
    ledcWrite(LEDC_CHANNEL_0, fadeValue);
  }
  else if (serialOtaTrigger) {
    // Digital blink every 250ms
    digitalWrite(PAIRING_LED_PIN, (millis() / 250) % 2);
  }
  else {
    ledcWrite(LEDC_CHANNEL_0, 0);    // LED off
     digitalWrite(PAIRING_LED_PIN, LOW);
  }
}

void clearPairingNVS() {
    nvs.begin(NVS_NAMESPACE, false);
    nvs.clear();
    nvs.putInt("version", STORAGE_VERSION);
    nvs.end();
    log(LOG_INFO, "Pairing info cleared from NVS");
}

void saveServerToNVS(const uint8_t* mac, uint8_t channel) {
    Preferences nvs;
    log(LOG_DEBUG, "Saving server info to NVS...");
    if (nvs.begin(NVS_NAMESPACE, false)) {
        nvs.putBytes("server_mac", mac, 6);
        nvs.putUChar("channel", channel);
        nvs.putInt("version", STORAGE_VERSION);
        log(LOG_INFO, "Server info saved to NVS:");
        printMAC(mac, LOG_INFO);
        log(LOG_INFO, "Channel: " + String(channel));
        nvs.end();
    } else {
        log(LOG_ERROR, "Failed to open NVS for writing!");
    }
}

bool loadServerFromNVS(uint8_t* mac, uint8_t* channel) {
    Preferences nvs;
    log(LOG_DEBUG, "Loading server info from NVS...");
    bool success = false;
    if (nvs.begin(NVS_NAMESPACE, true)) {
        if (nvs.getInt("version", 0) != STORAGE_VERSION) {
            log(LOG_WARN, "Incorrect NVS version, resetting NVS");
            nvs.end();
            clearPairingNVS();
            nvs.begin(NVS_NAMESPACE, false);
            nvs.putInt("version", STORAGE_VERSION);
            nvs.end();
            return success;
        }
        if (nvs.getBytesLength("server_mac") == 6) {
            nvs.getBytes("server_mac", mac, 6);
            *channel = nvs.getUChar("channel", 1);
            log(LOG_INFO, "Server info loaded from NVS:");
            printMAC(mac, LOG_INFO);
            log(LOG_INFO, "Channel: " + String(*channel));
            success = true;
        } else {
            log(LOG_DEBUG, "No server MAC found in NVS");
        }
        nvs.end();
    } else {
        log(LOG_ERROR, "Failed to open NVS for reading!");
    }
    return success;
}

void addPeer(const uint8_t* mac_addr, uint8_t chan) {
    log(LOG_DEBUG, "Adding peer to ESP-NOW...");
    
    // Set the WiFi channel
    ESP_ERROR_CHECK(esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE));
    log(LOG_DEBUG, "WiFi channel set to " + String(chan));

    // Remove the peer first to avoid duplicates
    esp_now_del_peer(mac_addr);

    // Prepare the peer info struct
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    peer.channel = chan;
    peer.encrypt = false;
    memcpy(peer.peer_addr, mac_addr, 6);

    // Try to add the peer
    if (esp_now_add_peer(&peer) == ESP_OK) {
        log(LOG_INFO, "Peer added successfully");
        printMAC(mac_addr, LOG_INFO);

        // Only save if this is a new or changed server
        bool shouldSave = false;
        for (int i = 0; i < 6; i++) {
            if (serverAddress[i] != mac_addr[i]) {
                shouldSave = true;
                break;
            }
        }
        if (!shouldSave && currentChannel != chan) {
            shouldSave = true;
        }

        // Update RAM copy in any case
        memcpy(serverAddress, mac_addr, 6);
        currentChannel = chan;

        if (shouldSave) {
            saveServerToNVS(mac_addr, chan);
            log(LOG_DEBUG, "Server info saved to NVS");
        } else {
            log(LOG_DEBUG, "Server info unchanged, not saving to NVS");
        }
    } else {
        log(LOG_ERROR, "Failed to add peer!");
        printMAC(mac_addr, LOG_ERROR);
    }
}

PairingStatus autoPairing(){
  switch(pairingStatus) {
    case PAIR_REQUEST:
      log(LOG_INFO, "Starting pairing on channel " + String(currentChannel));

      // set WiFi channel   
      ESP_ERROR_CHECK(esp_wifi_set_channel(currentChannel,  WIFI_SECOND_CHAN_NONE));
      initESP_NOW();
    
      // set pairing data to send to the server
      pairingData.msgType = PAIRING;
      pairingData.id = BOARD_ID;     
      pairingData.channel = currentChannel;
      pairingData.macAddr[0] = clientMacAddress[0];
      pairingData.macAddr[1] = clientMacAddress[1];
      pairingData.macAddr[2] = clientMacAddress[2];
      pairingData.macAddr[3] = clientMacAddress[3];
      pairingData.macAddr[4] = clientMacAddress[4];
      pairingData.macAddr[5] = clientMacAddress[5];
      strncpy(pairingData.name, deviceName, MAX_PEER_NAME_LEN);
      
      // add peer and send request
      addPeer(serverAddress, currentChannel);
      esp_now_send(serverAddress, (uint8_t *) &pairingData, sizeof(pairingData));
      log(LOG_DEBUG, "Pairing request sent");
      previousMillis = millis();
      pairingStatus = PAIR_REQUESTED;
      break;

    case PAIR_REQUESTED:
      // time out to allow receiving response from server
      currentMillis = millis();
      if(currentMillis - previousMillis > 1000) {
        previousMillis = currentMillis;
        // time out expired,  try next channel
        currentChannel ++;
        if (currentChannel > MAX_CHANNEL){
          currentChannel = 1;
        }   
        log(LOG_DEBUG, "Pairing timeout, trying channel " + String(currentChannel));
        pairingStatus = PAIR_REQUEST;
      }
    break;

    case PAIR_PAIRED:
      // nothing to do here 
    break;
  }
  return pairingStatus;
}  
