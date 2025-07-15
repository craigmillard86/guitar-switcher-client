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
#pragma once
#include "config.h"
#include "globals.h"
#include <Arduino.h>

// Enhanced logging functions
void log(LogLevel level, const String& msg);
void logf(LogLevel level, const char* format, ...);
void logWithTimestamp(LogLevel level, const String& msg);
void printMAC(const uint8_t *mac, LogLevel level);
void blinkLED(uint8_t pin, int times, int delayMs);
void printStruct(const struct_message& data);

// System status and debug functions
void printSystemStatus();
void printMemoryInfo();
void printNetworkStatus();
void printAmpChannelStatus();
void printPairingStatus();

// Enhanced serial command handling
void checkSerialCommands();
void printHelpMenu();
void handleSerialCommand(const String& cmd);

// Utility functions
String getLogLevelString(LogLevel level);
String getPairingStatusString(PairingStatus status);
String getUptimeString();
uint32_t getFreeHeap();
uint32_t getMinFreeHeap();

// NVS functions for log level persistence
void saveLogLevelToNVS(LogLevel level);
LogLevel loadLogLevelFromNVS();
void clearLogLevelNVS();

// Client configuration functions
void printClientConfiguration();
