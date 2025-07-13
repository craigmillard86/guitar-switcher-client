#pragma once
#include "globals.h"
#include <Arduino.h>

// Debug and monitoring functions
void printDebugInfo();
void printPerformanceMetrics();
void printTaskStats();
void printWiFiStats();
void printESPNowStats();

// Memory monitoring
void updateMemoryStats();
void printMemoryLeakInfo();

// Performance monitoring
struct PerformanceMetrics {
    unsigned long loopCount;
    unsigned long lastLoopTime;
    unsigned long maxLoopTime;
    unsigned long minLoopTime;
    unsigned long totalLoopTime;
    unsigned long startTime;
};

extern PerformanceMetrics perfMetrics;

// Debug commands
void handleDebugCommand(const String& cmd);
void printDebugHelp();

// Performance monitoring functions
void updatePerformanceMetrics(unsigned long loopTime); 