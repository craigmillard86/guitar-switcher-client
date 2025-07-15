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
#include "globals.h"


char deviceName[MAX_PEER_NAME_LEN] = {0};
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

uint8_t ampSwitchPins[MAX_AMPSWITCHS] = {0}; // Will be set at runtime
uint8_t ampButtonPins[MAX_AMPSWITCHS] = {0}; // Will be set at runtime
uint8_t currentAmpChannel = 0; // No channel active at startup

// Button control flag - set to false when buttons aren't connected
bool enableButtonChecking = false;

volatile StatusLedPattern currentLedPattern = LED_OFF;
volatile unsigned long ledPatternStart = 0;
volatile int ledPatternStep = 0;


