#pragma once
#include "globals.h"

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

#elif CLIENT_TYPE == CUSTOM
    #define CLIENT_TYPE_ENUM CLIENT_CUSTOM
    #define HAS_AMP_SWITCHING false

#else
    #error "Unknown CLIENT_TYPE specified"
#endif

// Device name configuration
#ifndef DEVICE_NAME
#define DEVICE_NAME "ESP32_CLIENT"
#endif

// Function declarations
uint8_t* parsePinArray(const char* pinString);
String getClientTypeString();
void printClientConfiguration();
void initializeClientConfiguration(); 