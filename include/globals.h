#pragma once
#include "dataStructs.h"
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#define FIRMWARE_VERSION "1.0.0"
#define STORAGE_VERSION 1

#define OTA_BUTTON_PIN 0          // Use existing button (e.g., GPIO0)
#define OTA_HOLD_TIME 2000
extern bool serialOtaTrigger;

#define BOARD_ID 1
#define MAX_CHANNEL 13
#define PAIRING_BUTTON_PIN 0

#define STATUS_LED_PIN 2
#define PAIRING_LED_BLINK 100
#define PAIRING_RETRY_DELAY 300

#define PAIRING_LED_PIN 2              // Use actual pin you're using
#define LEDC_CHANNEL_0  0              // LEDC channel (0–15)
#define LEDC_TIMER_13_BIT 13           // Resolution (0–8191)
#define LEDC_BASE_FREQ 1000            // Hz

#define NVS_NAMESPACE "pairing"
#define MAX_PEER_NAME_LEN 32

#define MAX_AMPSWITCHS 4 // <---- Set this to your channel count!
extern uint8_t currentAmpChannel;
extern uint8_t ampSwitchPins[MAX_AMPSWITCHS];
extern uint8_t ampButtonPins[MAX_AMPSWITCHS];
#define MIDI_RX_PIN 6   // ESP32-C3 pin for MIDI IN (RX)
#define MIDI_TX_PIN 7   // ESP32-C3 pin for MIDI OUT/THRU (TX)

enum PairingStatus {NOT_PAIRED, PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED,};
extern PairingStatus pairingStatus;

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
