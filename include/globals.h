#pragma once
#include "pairing.h"
// NOTE: If you need config macros, include config.h BEFORE globals.h in your source file.
#include "dataStructs.h"
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#define FIRMWARE_VERSION "1.0.0"
#define STORAGE_VERSION 1

extern bool serialOtaTrigger;

#define BOARD_ID 1

extern uint8_t currentAmpChannel;
extern uint8_t ampSwitchPins[MAX_AMPSWITCHS];
extern uint8_t ampButtonPins[MAX_AMPSWITCHS];

// PairingStatus now in pairing.h

enum LogLevel {
  LOG_NONE = 0,
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG
};

extern LogLevel currentLogLevel;

extern uint8_t serverAddress[6];
extern uint8_t clientMacAddress[6];
extern esp_now_peer_info_t peer;
extern struct_message myData;
extern struct_message inData;
extern struct_pairing pairingData;
extern uint8_t currentChannel;
extern bool paired;
extern char deviceName[MAX_PEER_NAME_LEN];
extern bool newDataReceived;
extern bool otaModeRequested;
extern bool enableButtonChecking;
