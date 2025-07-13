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

