#include "espnow.h"
#include "globals.h"
#include "utils.h"
#include "espnow-pairing.h"
#include "commandHandler.h"
#include <esp_now.h>
#include <WiFi.h>
#include <espnow-pairing.h>

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  log(LOG_DEBUG,"\r\nLast Packet Send Status:\t");
  log(LOG_DEBUG,status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
    uint8_t type = incomingData[0];
    if (pairingStatus != PAIR_PAIRED && type != PAIRING) {
    log(LOG_INFO, "[Client] Ignoring data: not paired.");
    return;
    }
    log(LOG_DEBUG,"Packet received with ");
    log(LOG_DEBUG,"data size = ");
    log(LOG_DEBUG,String(sizeof(incomingData)));
    switch (type) {
    case DATA :      // we received data from server
    memcpy(&inData, incomingData, sizeof(inData));
    log(LOG_DEBUG,"ID  = " + String(inData.id));
    log(LOG_DEBUG,"Setpoint temp = "+ String(inData.temp));
    log(LOG_DEBUG,"SetPoint humidity = "+ String(inData.hum));
    log(LOG_DEBUG,"reading Id  = "+ String(inData.readingId));

    if (inData.readingId % 2 == 1){
      digitalWrite(LED_BUILTIN, LOW);
    } else { 
      digitalWrite(LED_BUILTIN, HIGH);
    }
    break;

  case PAIRING:    // we received pairing data from server
    memcpy(&pairingData, incomingData, sizeof(pairingData));
    if (pairingData.id == 0) {              // the message comes from server
      log(LOG_ERROR,"Pairing done for MAC Address: ");
      printMAC(pairingData.macAddr, LOG_INFO);
      log(LOG_ERROR,"on channel " + String(pairingData.channel) + " in " + String(millis()-start) + "ms");
      addPeer(pairingData.macAddr, pairingData.channel); // add the server  to the peer list 
      pairingStatus = PAIR_PAIRED;             // set the pairing status
    }
    break;

    case COMMAND:
        log(LOG_INFO, "[Client] Command received from server");
        handleCommand(inData.commandType, inData.commandValue);
    break;
  }  
}

void initESP_NOW(){
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      log(LOG_ERROR,"Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
} 