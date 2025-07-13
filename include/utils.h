#pragma once
#include "dataStructs.h"
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
