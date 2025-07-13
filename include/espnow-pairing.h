#pragma once
#include "dataStructs.h"
#include <Arduino.h>
#include <globals.h>

extern unsigned long currentMillis;
extern unsigned long previousMillis;   // Stores last time temperature was published
extern const long interval;        // Interval at which to publish sensor readings
extern unsigned long start;                // used to measure Pairing time
extern unsigned int readingId;  

void startPairing();
void saveServerToNVS(const uint8_t* mac, uint8_t channel);
bool loadServerFromNVS(uint8_t* mac, uint8_t* channel);
void clearPairingNVS();
void checkPairingButton();
void updatePairingLED();
PairingStatus autoPairing();
void addPeer(const uint8_t * mac_addr, uint8_t chan);
