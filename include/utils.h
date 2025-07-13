#pragma once
#include "dataStructs.h"
#include "globals.h"
#include <Arduino.h>

void log(LogLevel level, const String& msg);
void printMAC(const uint8_t *mac, LogLevel level);
void blinkLED(uint8_t pin, int times, int delayMs);
void printStruct(const struct_message& data);
void checkSerialCommands();
