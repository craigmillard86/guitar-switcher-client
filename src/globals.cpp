#include <Arduino.h>
#include "config.h"
#include "pairing.h"
#include "globals.h"


char deviceName[MAX_PEER_NAME_LEN] = "CLIENT_1";
LogLevel currentLogLevel = LOG_DEBUG;
esp_now_peer_info_t peer = {};
struct_message myData = {};
struct_message inData = {};
struct_pairing pairingData = {};
PairingStatus pairingStatus = NOT_PAIRED;
bool serialOtaTrigger = false;

uint8_t serverAddress[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
uint8_t clientMacAddress[6] = {0};
uint8_t currentChannel = 4;

uint8_t ampSwitchPins[MAX_AMPSWITCHS] = {4, 5, 6, 7}; // GPIO pin numbers
uint8_t ampButtonPins[MAX_AMPSWITCHS] = {4, 5, 6, 7}; // GPIO pin numbers
uint8_t currentAmpChannel = 0; // No channel active at startup

// Button control flag - set to false when buttons aren't connected
bool enableButtonChecking = false;


