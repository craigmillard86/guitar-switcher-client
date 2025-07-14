#include <Arduino.h>
#pragma once

// Client Type Definitions
enum ClientType {
    CLIENT_AMP_SWITCHER,
    CLIENT_CUSTOM
};

// Default client type if not specified
#ifndef CLIENT_TYPE
#define CLIENT_TYPE AMP_SWITCHER
#endif

// Client Type Configuration
#if CLIENT_TYPE == AMP_SWITCHER
    #define CLIENT_TYPE_ENUM CLIENT_AMP_SWITCHER
    #define HAS_AMP_SWITCHING true
    
    #ifndef MAX_AMPSWITCHS
    #define MAX_AMPSWITCHS 4
    #endif
    
    #ifndef AMP_SWITCH_PINS
    #define AMP_SWITCH_PINS "4,5,6,7"
    #endif
    
    #ifndef AMP_BUTTON_PINS
    #define AMP_BUTTON_PINS "8,9,10,11"
    #endif

#else // CUSTOM
    #define CLIENT_TYPE_ENUM CLIENT_CUSTOM
    #define HAS_AMP_SWITCHING false
#endif

// Device name configuration
#ifndef DEVICE_NAME
#define DEVICE_NAME "ESP32_CLIENT"
#endif

// Pin assignments and hardware config
#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 2
#endif
#ifndef PAIRING_LED_PIN
#define PAIRING_LED_PIN 2
#endif
#ifndef PAIRING_BUTTON_PIN
#define PAIRING_BUTTON_PIN 0
#endif
#ifndef OTA_BUTTON_PIN
#define OTA_BUTTON_PIN 0
#endif
#ifndef OTA_HOLD_TIME
#define OTA_HOLD_TIME 2000
#endif
#ifndef MIDI_RX_PIN
#define MIDI_RX_PIN 6
#endif
#ifndef MIDI_TX_PIN
#define MIDI_TX_PIN 7
#endif
#ifndef LEDC_CHANNEL_0
#define LEDC_CHANNEL_0 0
#endif
#ifndef LEDC_TIMER_13_BIT
#define LEDC_TIMER_13_BIT 13
#endif
#ifndef LEDC_BASE_FREQ
#define LEDC_BASE_FREQ 1000
#endif
#ifndef PAIRING_LED_BLINK
#define PAIRING_LED_BLINK 100
#endif
#ifndef PAIRING_RETRY_DELAY
#define PAIRING_RETRY_DELAY 300
#endif
#ifndef MAX_CHANNEL
#define MAX_CHANNEL 13
#endif
#ifndef MAX_PEER_NAME_LEN
#define MAX_PEER_NAME_LEN 32
#endif
#ifndef NVS_NAMESPACE
#define NVS_NAMESPACE "pairing"
#endif

// Function declarations
uint8_t* parsePinArray(const char* pinString);
String getClientTypeString();
void printClientConfiguration();
void initializeClientConfiguration(); 