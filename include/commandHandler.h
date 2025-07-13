#pragma once
#include "dataStructs.h"
#include <Arduino.h>

void handleCommand(uint8_t commandType, uint8_t value);
void setAmpChannel(uint8_t channel);
void checkAmpChannelButtons();
void handleProgramChange(byte midiChannel, byte program);