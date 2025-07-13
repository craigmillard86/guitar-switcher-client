#pragma once
#include "dataStructs.h"
#include <Arduino.h>
#include <esp_now.h>

void setupEspNow();
void sendData();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) ;
void initESP_NOW();
