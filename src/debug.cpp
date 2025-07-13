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
    log(LOG_INFO, "  Loop Count: " + String(perfMetrics.loopCount));
    log(LOG_INFO, "  Last Loop Time: " + String(perfMetrics.lastLoopTime) + "ms");
    log(LOG_INFO, "  Max Loop Time: " + String(perfMetrics.maxLoopTime) + "ms");
    log(LOG_INFO, "  Min Loop Time: " + String(perfMetrics.minLoopTime) + "ms");
    log(LOG_INFO, "  Avg Loop Time: " + String(avgLoopTime, 2) + "ms");
    log(LOG_INFO, "  Uptime: " + String(uptime) + "ms");
}

void printTaskStats() {
    log(LOG_INFO, "Task Statistics:");
    log(LOG_INFO, "  Free Stack: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");
    log(LOG_INFO, "  CPU Usage: " + String(100 - (getFreeHeap() * 100 / heap_caps_get_total_size(MALLOC_CAP_8BIT)), 1) + "%");
}

void printWiFiStats() {
    log(LOG_INFO, "WiFi Statistics:");
    log(LOG_INFO, "  Mode: " + String(WiFi.getMode()));
    log(LOG_INFO, "  Channel: " + String(currentChannel));
    log(LOG_INFO, "  RSSI: " + String(WiFi.RSSI()) + " dBm");
    log(LOG_INFO, "  Power Mode: Active");
}

void printESPNowStats() {
    log(LOG_INFO, "ESP-NOW Statistics:");
    log(LOG_INFO, "  Pairing Status: " + getPairingStatusString(pairingStatus));
    log(LOG_INFO, "  Peers: 1");
    log(LOG_INFO, "  Max Peers: 20");
}

void updateMemoryStats() {
    uint32_t currentFreeHeap = getFreeHeap();
    
    if (initialFreeHeap == 0) {
        initialFreeHeap = currentFreeHeap;
    }
    
    if (currentFreeHeap < lastFreeHeap) {
        log(LOG_DEBUG, "Memory decreased: " + String(lastFreeHeap - currentFreeHeap) + "B");
    }
    
    lastFreeHeap = currentFreeHeap;
    lastMemoryCheck = millis();
}

void printMemoryLeakInfo() {
    uint32_t currentFreeHeap = getFreeHeap();
    int32_t memoryChange = currentFreeHeap - initialFreeHeap;
    
    log(LOG_INFO, "Memory Leak Analysis:");
    log(LOG_INFO, "  Initial Free Heap: " + String(initialFreeHeap) + "B");
    log(LOG_INFO, "  Current Free Heap: " + String(currentFreeHeap) + "B");
    log(LOG_INFO, "  Memory Change: " + String(memoryChange) + "B");
    
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