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
#include "debug.h"
#include "utils.h"
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_task_wdt.h>

// Global performance metrics
PerformanceMetrics perfMetrics = {0};

// Memory tracking
uint32_t initialFreeHeap = 0;
uint32_t lastFreeHeap = 0;
unsigned long lastMemoryCheck = 0;

void printDebugInfo() {
    log(LOG_INFO, "=== DEBUG INFORMATION ===");
    printPerformanceMetrics();
    printMemoryInfo();
    printWiFiStats();
    printESPNowStats();
    log(LOG_INFO, "========================");
}

void printPerformanceMetrics() {
    unsigned long uptime = millis() - perfMetrics.startTime;
    float avgLoopTime = perfMetrics.loopCount > 0 ? (float)perfMetrics.totalLoopTime / perfMetrics.loopCount : 0;
    
    log(LOG_INFO, "Performance Metrics:");
    logf(LOG_INFO, "  Loop Count: %lu", perfMetrics.loopCount);
    logf(LOG_INFO, "  Last Loop Time: %lums", perfMetrics.lastLoopTime);
    logf(LOG_INFO, "  Max Loop Time: %lums", perfMetrics.maxLoopTime);
    logf(LOG_INFO, "  Min Loop Time: %lums", perfMetrics.minLoopTime);
    logf(LOG_INFO, "  Avg Loop Time: %.2fms", avgLoopTime);
    logf(LOG_INFO, "  Uptime: %lums", uptime);
}

void printTaskStats() {
    log(LOG_INFO, "Task Statistics:");
    logf(LOG_INFO, "  Free Stack: %lu bytes", uxTaskGetStackHighWaterMark(NULL));
    logf(LOG_INFO, "  CPU Usage: %.1f%%", 100 - (getFreeHeap() * 100 / heap_caps_get_total_size(MALLOC_CAP_8BIT)));
}

void printWiFiStats() {
    log(LOG_INFO, "WiFi Statistics:");
    logf(LOG_INFO, "  Mode: %d", WiFi.getMode());
    logf(LOG_INFO, "  Channel: %u", currentChannel);
    logf(LOG_INFO, "  RSSI: %d dBm", WiFi.RSSI());
    log(LOG_INFO, "  Power Mode: Active");
}

void printESPNowStats() {
    log(LOG_INFO, "ESP-NOW Statistics:");
    logf(LOG_INFO, "  Pairing Status: %s", getPairingStatusString(pairingStatus));
    log(LOG_INFO, "  Peers: 1");
    log(LOG_INFO, "  Max Peers: 20");
}

void updateMemoryStats() {
    uint32_t currentFreeHeap = getFreeHeap();
    
    if (initialFreeHeap == 0) {
        initialFreeHeap = currentFreeHeap;
    }
    
    if (currentFreeHeap < lastFreeHeap) {
        logf(LOG_DEBUG, "Memory decreased: %luB", lastFreeHeap - currentFreeHeap);
    }
    
    lastFreeHeap = currentFreeHeap;
    lastMemoryCheck = millis();
}

void printMemoryLeakInfo() {
    uint32_t currentFreeHeap = getFreeHeap();
    int32_t memoryChange = currentFreeHeap - initialFreeHeap;
    
    log(LOG_INFO, "Memory Leak Analysis:");
    logf(LOG_INFO, "  Initial Free Heap: %luB", initialFreeHeap);
    logf(LOG_INFO, "  Current Free Heap: %luB", currentFreeHeap);
    logf(LOG_INFO, "  Memory Change: %ldB", memoryChange);
    
    if (memoryChange < -1000) {
        log(LOG_WARN, "  Potential memory leak detected!");
    } else if (memoryChange > 1000) {
        log(LOG_INFO, "  Memory freed");
    } else {
        log(LOG_INFO, "  Memory stable");
    }
}

void handleDebugCommand(const String& cmd) {
    if (cmd.equalsIgnoreCase("debug")) {
        printDebugInfo();
    } else if (cmd.equalsIgnoreCase("perf")) {
        printPerformanceMetrics();
    } else if (cmd.equalsIgnoreCase("memory")) {
        printMemoryInfo();
        printMemoryLeakInfo();
    } else if (cmd.equalsIgnoreCase("wifi")) {
        printWiFiStats();
    } else if (cmd.equalsIgnoreCase("espnow")) {
        printESPNowStats();
    } else if (cmd.equalsIgnoreCase("task")) {
        printTaskStats();
    } else if (cmd.equalsIgnoreCase("debughelp")) {
        printDebugHelp();
    } else {
        log(LOG_WARN, "Unknown debug command: '" + cmd + "'");
        log(LOG_INFO, "Type 'debughelp' for debug commands");
    }
}

void printDebugHelp() {
    Serial.println(F("\n========== DEBUG COMMANDS =========="));
    Serial.println(F("debug       : Show complete debug information"));
    Serial.println(F("perf        : Show performance metrics"));
    Serial.println(F("memory      : Show memory usage and leak analysis"));
    Serial.println(F("wifi        : Show WiFi statistics"));
    Serial.println(F("espnow      : Show ESP-NOW statistics"));
    Serial.println(F("task        : Show task statistics"));
    Serial.println(F("debughelp   : Show this debug help"));
    Serial.println(F("=====================================\n"));
}

// Performance monitoring functions
void updatePerformanceMetrics(unsigned long loopTime) {
    perfMetrics.loopCount++;
    perfMetrics.lastLoopTime = loopTime;
    perfMetrics.totalLoopTime += loopTime;
    
    if (loopTime > perfMetrics.maxLoopTime) {
        perfMetrics.maxLoopTime = loopTime;
    }
    
    if (perfMetrics.minLoopTime == 0 || loopTime < perfMetrics.minLoopTime) {
        perfMetrics.minLoopTime = loopTime;
    }
} 