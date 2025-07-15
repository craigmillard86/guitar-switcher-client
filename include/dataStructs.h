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
#include <Arduino.h>

#define MAX_PEER_NAME_LEN 32

enum MessageType { PAIRING, DATA, COMMAND };
enum CommandType { PROGRAM_CHANGE, CHANNEL_2_STATUS, CHANNEL_3_STATUS, CHANNEL_4_STATUS };
typedef struct struct_message {
    uint8_t msgType;
    uint8_t id;
    float temp;
    float hum;
    unsigned int readingId;
    uint8_t commandType;
    uint8_t commandValue;
} struct_message;

typedef struct struct_pairing {
    uint8_t msgType;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
    char name[MAX_PEER_NAME_LEN];
} struct_pairing;

