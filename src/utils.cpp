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
//
#include <Arduino.h>
#include "utils.h"
#include <espnow-pairing.h>
#include <commandHandler.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include "debug.h"
#include "nvsManager.h"

extern unsigned long lastMemoryCheck;

// Global variables for memory tracking
uint32_t minFreeHeap = UINT32_MAX;
// unsigned long lastMemoryCheck = 0; // Removed to avoid multiple definition

// Enhanced logging with timestamps and proper log levels - Memory optimized
void log(LogLevel level, const String& msg) {
    if (level <= currentLogLevel) {
        char timestamp[32];
        getUptimeString(timestamp, sizeof(timestamp));
        const char* levelStr = getLogLevelString(level);
        Serial.printf("[%s][%s] %s\n", timestamp, levelStr, msg.c_str());
    }
}

void logf(LogLevel level, const char* format, ...) {
    if (level <= currentLogLevel) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        char timestamp[32];
        getUptimeString(timestamp, sizeof(timestamp));
        const char* levelStr = getLogLevelString(level);
        Serial.printf("[%s][%s] %s\n", timestamp, levelStr, buffer);
    }
}

void logWithTimestamp(LogLevel level, const String& msg) {
    log(level, msg);
}

void printMAC(const uint8_t *mac, LogLevel level) {
    if (level <= currentLogLevel) {
        char timestamp[32];
        getUptimeString(timestamp, sizeof(timestamp));
        const char* levelStr = getLogLevelString(level);
        Serial.printf("[%s][%s] %02X:%02X:%02X:%02X:%02X:%02X\n", 
                     timestamp, levelStr,
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
}

void blinkLED(uint8_t pin, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        digitalWrite(pin, HIGH);
        delay(delayMs);
        digitalWrite(pin, LOW);
        delay(delayMs);
    }
}

// System status functions
void printSystemStatus() {
    log(LOG_INFO, "=== SYSTEM STATUS ===");
    logf(LOG_INFO, "Firmware Version: %s", FIRMWARE_VERSION);
    logf(LOG_INFO, "Board ID: %d", BOARD_ID);
    
    char uptime[32];
    getUptimeString(uptime, sizeof(uptime));
    logf(LOG_INFO, "Uptime: %s", uptime);
    
    printMemoryInfo();
    printNetworkStatus();
    printAmpChannelStatus();
    printPairingStatus();
    log(LOG_INFO, "===================");
}

void printMemoryInfo() {
    uint32_t freeHeap = getFreeHeap();
    uint32_t totalHeap = heap_caps_get_total_size(MALLOC_CAP_8BIT);
    uint32_t usedHeap = totalHeap - freeHeap;
    float usagePercent = (float)usedHeap / totalHeap * 100.0;
    
    logf(LOG_INFO, "Memory - Free: %uB, Used: %uB (%.1f%%)", freeHeap, usedHeap, usagePercent);
    logf(LOG_INFO, "Min Free Heap: %uB", minFreeHeap);
}

void printNetworkStatus() {
    logf(LOG_INFO, "WiFi Mode: %d", WiFi.getMode());
    logf(LOG_INFO, "Current Channel: %u", currentChannel);
    log(LOG_INFO, "Client MAC: ");
    printMAC(clientMacAddress, LOG_INFO);
    log(LOG_INFO, "Server MAC: ");
    printMAC(serverAddress, LOG_INFO);
}

void printAmpChannelStatus() {
    logf(LOG_INFO, "Current Amp Channel: %u", currentAmpChannel);
    logf(LOG_INFO, "Channel Pins: %u,%u,%u,%u", 
         ampSwitchPins[0], ampSwitchPins[1], ampSwitchPins[2], ampSwitchPins[3]);
    logf(LOG_INFO, "Button Pins: %u,%u,%u,%u", 
         ampButtonPins[0], ampButtonPins[1], ampButtonPins[2], ampButtonPins[3]);
}

void printPairingStatus() {
    logf(LOG_INFO, "Pairing Status: %s", getPairingStatusString(pairingStatus));
}

// Enhanced serial command handling
void checkSerialCommands() {
    if (midiLearnChannel >= 0) return; // Block serial commands during MIDI Learn lockout
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        handleSerialCommand(cmd);
    }
}

void handleSerialCommand(const String& cmd) {
    if (cmd.isEmpty()) return;

    setStatusLedPattern(LED_DOUBLE_FLASH);
    // System commands
    if (cmd.equalsIgnoreCase("help")) {
        printHelpMenu();
    } else if (cmd.equalsIgnoreCase("status")) {
        printSystemStatus();
    } else if (cmd.equalsIgnoreCase("memory")) {
        printMemoryInfo();
    } else if (cmd.equalsIgnoreCase("network")) {
        printNetworkStatus();
    } else if (cmd.equalsIgnoreCase("amp")) {
        printAmpChannelStatus();
    } else if (cmd.equalsIgnoreCase("pairing")) {
        printPairingStatus();
    } else if (cmd.equalsIgnoreCase("pins")) {
        log(LOG_INFO, "=== PIN ASSIGNMENTS ===");
        
        // Build pin strings using char arrays to avoid String concatenation
        char switchPinsStr[32] = "";
        char buttonPinsStr[32] = "";
        
        for (int i = 0; i < MAX_AMPSWITCHS; i++) {
            char pinStr[8];
            snprintf(pinStr, sizeof(pinStr), "%u", ampSwitchPins[i]);
            if (i > 0) strcat(switchPinsStr, ",");
            strcat(switchPinsStr, pinStr);
            
            snprintf(pinStr, sizeof(pinStr), "%u", ampButtonPins[i]);
            if (i > 0) strcat(buttonPinsStr, ",");
            strcat(buttonPinsStr, pinStr);
        }
        
        logf(LOG_INFO, "Amp Switch Pins: %s", switchPinsStr);
        logf(LOG_INFO, "Amp Button Pins: %s", buttonPinsStr);
        logf(LOG_INFO, "Status/Pairing LED Pin: %u", PAIRING_LED_PIN);
        logf(LOG_INFO, "MIDI RX Pin: %u", MIDI_RX_PIN);
        logf(LOG_INFO, "MIDI TX Pin: %u", MIDI_TX_PIN);
        log(LOG_INFO, "======================");
    } else if (cmd.equalsIgnoreCase("uptime")) {
        char uptime[32];
        getUptimeString(uptime, sizeof(uptime));
        logf(LOG_INFO, "Uptime: %s", uptime);
    } else if (cmd.equalsIgnoreCase("restart") || cmd.equalsIgnoreCase("reset")) {
        log(LOG_WARN, "Restarting ESP32...");
        delay(1000);
        ESP.restart();
    } else if (cmd.equalsIgnoreCase("ota")) {
        serialOtaTrigger = true;
        log(LOG_INFO, "OTA mode triggered");
    } else if (cmd.equalsIgnoreCase("pair")) {
        clearPairingNVS();
        pairingStatus = PAIR_REQUEST;
        log(LOG_INFO, "Re-pairing requested!");
    } else if (cmd.startsWith("setlog")) {
        int level = cmd.substring(6).toInt();
        if (level >= 0 && level <= 4) {
            currentLogLevel = (LogLevel)level;
            saveLogLevelToNVS(currentLogLevel); // Save the new log level
            logf(LOG_INFO, "Log level set to: %s", getLogLevelString(currentLogLevel));
        } else {
            log(LOG_WARN, "Invalid log level. Use 0-4 (0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG)");
        }
    } else if (cmd.equalsIgnoreCase("testled")) {
        log(LOG_INFO, "Testing status LED...");
        //blinkLED(STATUS_LED_PIN, 3, 200);
        setStatusLedPattern(LED_TRIPLE_FLASH);
    } else if (cmd.equalsIgnoreCase("testpairing")) {
        log(LOG_INFO, "Testing pairing LED...");
        for (int i = 0; i < 5; i++) {
            ledcWrite(LEDC_CHANNEL_0, 512);
            delay(100);
            ledcWrite(LEDC_CHANNEL_0, 0);
            delay(100);
        }
    } else if (cmd.equalsIgnoreCase("off")) {
        setAmpChannel(0);
        log(LOG_INFO, "All amp channels turned off");
    } else if (cmd.toInt() > 0 && cmd.toInt() <= MAX_AMPSWITCHS) {
        uint8_t ch = cmd.toInt();
        setAmpChannel(ch);
        logf(LOG_INFO, "Amp channel set to %u", ch);
    } else if (cmd.length() == 2 && cmd[0] == 'b') {
        int btn = cmd[1] - '0';
        if (btn >= 1 && btn <= MAX_AMPSWITCHS) {
            logf(LOG_INFO, "Simulating button %d press", btn);
            setAmpChannel(btn);
        } else {
            logf(LOG_WARN, "Invalid button number. Use b1-b%d", MAX_AMPSWITCHS);
        }
    } else if (cmd.equalsIgnoreCase("midi")) {
        log(LOG_INFO, "=== MIDI INFORMATION ===");
        logf(LOG_INFO, "  Current MIDI Channel: %u (persistent, set via channel select mode)", currentMidiChannel);
        log(LOG_INFO, "  MIDI Thru: Enabled");
        logf(LOG_INFO, "  MIDI Pins - RX: %u, TX: %u", MIDI_RX_PIN, MIDI_TX_PIN);
        log(LOG_INFO, "  Program Change Mapping:");
        for (int i = 0; i < MAX_AMPSWITCHS; i++) {
            logf(LOG_INFO, "    Button %d: PC#%u", i+1, midiChannelMap[i]);
        }
        log(LOG_INFO, "  (Use 'chset' to change MIDI channel, 'midimap' for detailed mapping)");
    } else if (cmd.equalsIgnoreCase("version")) {
        logf(LOG_INFO, "Firmware Version: %s", FIRMWARE_VERSION);
        logf(LOG_INFO, "Storage Version: %d", STORAGE_VERSION);
    } else if (cmd.equalsIgnoreCase("buttons")) {
        enableButtonChecking = !enableButtonChecking;
        logf(LOG_INFO, "Button checking %s", enableButtonChecking ? "enabled" : "disabled");
    } else if (cmd.equalsIgnoreCase("loglevel")) {
        logf(LOG_INFO, "Current log level: %s (%u)", getLogLevelString(currentLogLevel), (uint8_t)currentLogLevel);
    } else if (cmd.equalsIgnoreCase("config")) {
        printClientConfiguration();
    } else if (cmd.equalsIgnoreCase("clearlog")) {
        clearLogLevelNVS();
        currentLogLevel = LOG_INFO; // Reset to default
        log(LOG_INFO, "Log level reset to default (INFO)");
    } else if (cmd.equalsIgnoreCase("clearall")) {
        log(LOG_WARN, "Clearing all NVS data...");
        clearPairingNVS();
        clearLogLevelNVS();
        currentLogLevel = LOG_INFO; // Reset to default
        pairingStatus = NOT_PAIRED;
        log(LOG_INFO, "All NVS data cleared - pairing and log level reset to defaults");
    } else if (cmd.startsWith("debug")) {
        String debugCmd = cmd.substring(5);
        debugCmd.trim();
        if (debugCmd.isEmpty()) {
            debugCmd = "debug"; // Default to show all debug info
        }
        handleDebugCommand(debugCmd);
    } else if (cmd.equalsIgnoreCase("midimap")) {
        log(LOG_INFO, "=== MIDI PROGRAM CHANGE MAP ===");
        for (int i = 0; i < MAX_AMPSWITCHS; i++) {
            logf(LOG_INFO, "Button %d: PC#%u", i+1, midiChannelMap[i]);
        }
        log(LOG_INFO, "==============================");
    } else if (cmd.equalsIgnoreCase("ch")) {
        logf(LOG_INFO, "Current MIDI Channel: %u (persistent, set via channel select mode)", currentMidiChannel);
    } else if (cmd.equalsIgnoreCase("chset")) {
        log(LOG_INFO, "To change MIDI channel: Hold Button 1 for 15s to enter channel select mode, then press to increment channel. Auto-saves after 10s of inactivity.");
    } else {
        logf(LOG_WARN, "Unknown command: '%s'", cmd.c_str());
        log(LOG_INFO, "Type 'help' for available commands");
    }
}

void printHelpMenu() {
    Serial.println(F("\n========== SERIAL COMMANDS =========="));
    Serial.println(F("SYSTEM COMMANDS:"));
    Serial.println(F("  help        : Show this help menu"));
    Serial.println(F("  status      : Show complete system status"));
    Serial.println(F("  memory      : Show memory usage"));
    Serial.println(F("  network     : Show network status"));
    Serial.println(F("  amp         : Show amp channel status"));
    Serial.println(F("  pairing     : Show pairing status"));
    Serial.println(F("  pins        : Show pin assignments (amp, button, LED, MIDI)"));
    Serial.println(F("  uptime      : Show system uptime"));
    Serial.println(F("  version     : Show firmware version"));
    Serial.println(F("  buttons     : Toggle button checking on/off"));
    Serial.println(F("  loglevel    : Show current log level"));
    Serial.println(F("  clearlog    : Clear saved log level (reset to default)"));
    Serial.println(F(""));
    Serial.println(F("MIDI COMMANDS:"));
    Serial.println(F("  midi        : Show current MIDI configuration and channel"));
    Serial.println(F("  midimap     : Show MIDI Program Change to channel mapping"));
    Serial.println(F("  ch          : Show the current MIDI channel (persistent, set via channel select mode)"));
    Serial.println(F("  chset       : Print instructions for entering channel select mode"));
    Serial.println(F(""));
    Serial.println(F("CONTROL COMMANDS:"));
    Serial.println(F("  restart     : Reboot the device"));
    Serial.println(F("  ota         : Enter OTA update mode"));
    Serial.println(F("  pair        : Clear pairing and re-pair"));
    Serial.println(F("  setlogN     : Set log level (N=0-4)"));
    Serial.println(F("  clearall    : Clear all NVS data (pairing + log level)"));
    Serial.println(F(""));
    Serial.println(F("TEST COMMANDS:"));
    Serial.println(F("  testled     : Test status LED"));
    Serial.println(F("  testpairing : Test pairing LED"));
    Serial.println(F(""));
    Serial.println(F("DEBUG COMMANDS:"));
    Serial.println(F("  debug       : Show complete debug info"));
    Serial.println(F("  debugperf   : Show performance metrics"));
    Serial.println(F("  debugmemory : Show memory analysis"));
    Serial.println(F("  debugwifi   : Show WiFi stats"));
    Serial.println(F("  debugespnow : Show ESP-NOW stats"));
    Serial.println(F("  debugtask   : Show task stats"));
    Serial.println(F("  debughelp   : Show debug commands"));
    Serial.println(F(""));
    Serial.println(F("AMP CHANNEL COMMANDS:"));
    Serial.println(F("  1-4         : Switch to amp channel 1-4"));
    Serial.println(F("  b1-b4       : Simulate button press 1-4"));
    Serial.println(F("  off         : Turn all channels off"));
    Serial.println(F(""));
    Serial.println(F("LOG LEVELS:"));
    Serial.println(F("  0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG"));
    Serial.println(F(""));
    Serial.println(F("EXAMPLES:"));
    Serial.println(F("  setlog3     : Show info and above logs"));
    Serial.println(F("  2           : Switch to channel 2"));
    Serial.println(F("  b3          : Simulate button 3 press"));
    Serial.println(F("  status      : Show system status"));
    Serial.println(F("  debug       : Show debug information"));
    Serial.println(F("=====================================\n"));
}

// Utility functions - Memory optimized
const char* getLogLevelString(LogLevel level) {
    switch (level) {
        case LOG_NONE: return "NONE";
        case LOG_ERROR: return "ERROR";
        case LOG_WARN: return "WARN";
        case LOG_INFO: return "INFO";
        case LOG_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

const char* getPairingStatusString(PairingStatus status) {
    switch (status) {
        case NOT_PAIRED: return "NOT_PAIRED";
        case PAIR_REQUEST: return "PAIR_REQUEST";
        case PAIR_REQUESTED: return "PAIR_REQUESTED";
        case PAIR_PAIRED: return "PAIR_PAIRED";
        default: return "UNKNOWN";
    }
}

void getUptimeString(char* buffer, size_t bufferSize) {
    unsigned long uptime = millis();
    unsigned long seconds = uptime / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    if (days > 0) {
        snprintf(buffer, bufferSize, "%lud %02lu:%02lu:%02lu", days, hours % 24, minutes % 60, seconds % 60);
    } else if (hours > 0) {
        snprintf(buffer, bufferSize, "%02lu:%02lu:%02lu", hours, minutes % 60, seconds % 60);
    } else {
        snprintf(buffer, bufferSize, "%02lu:%02lu", minutes, seconds % 60);
    }
}

uint32_t getFreeHeap() {
    uint32_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    if (freeHeap < minFreeHeap) {
        minFreeHeap = freeHeap;
    }
    return freeHeap;
}

uint32_t getMinFreeHeap() {
    return minFreeHeap;
}

void setStatusLedPattern(StatusLedPattern pattern) {
    // If pairing or OTA mode is active, ignore other patterns
    if (pairingStatus == PAIR_REQUEST || pairingStatus == PAIR_REQUESTED) {
        currentLedPattern = LED_FADE;
        return;
    }
    if (serialOtaTrigger) {
        currentLedPattern = LED_FAST_BLINK;
        return;
    }
    currentLedPattern = pattern;
    ledPatternStart = millis();
    ledPatternStep = 0;
}

void updateStatusLED() {
    static uint16_t fadeValue = 0;
    static int8_t fadeDirection = 1;
    static unsigned long lastUpdate = 0;
    static bool ledState = LOW;
    static unsigned long lastFadeUpdate = 0;
    unsigned long now = millis();

    // Only override patterns during active pairing phases, not when paired
    if (pairingStatus == PAIR_REQUEST || pairingStatus == PAIR_REQUESTED) {
        if (currentLedPattern != LED_FADE) {
            currentLedPattern = LED_FADE;
            ledPatternStart = now;
            ledPatternStep = 0;
        }
    } else if (serialOtaTrigger) {
        if (currentLedPattern != LED_FAST_BLINK) {
            currentLedPattern = LED_FAST_BLINK;
            ledPatternStart = now;
            ledPatternStep = 0;
        }
    }

    switch (currentLedPattern) {
        case LED_SINGLE_FLASH:
            if (ledPatternStep == 0) {
                ledcWrite(LEDC_CHANNEL_0, 8191);
                if (now - ledPatternStart > 80) { ledPatternStep = 1; ledPatternStart = now; }
            } else if (ledPatternStep == 1) {
                ledcWrite(LEDC_CHANNEL_0, 0);
                if (now - ledPatternStart > 120) { currentLedPattern = LED_OFF; }
            }
            break;
        case LED_DOUBLE_FLASH:
            if (ledPatternStep == 0 || ledPatternStep == 2) {
                ledcWrite(LEDC_CHANNEL_0, 8191);
                if (now - ledPatternStart > 60) { ledPatternStep++; ledPatternStart = now; }
            } else if (ledPatternStep == 1 || ledPatternStep == 3) {
                ledcWrite(LEDC_CHANNEL_0, 0);
                if (now - ledPatternStart > 60) { ledPatternStep++; ledPatternStart = now; }
            } else {
                currentLedPattern = LED_OFF;
            }
            break;
        case LED_TRIPLE_FLASH:
            if (ledPatternStep % 2 == 0) {
                ledcWrite(LEDC_CHANNEL_0, 8191);
            } else {
                ledcWrite(LEDC_CHANNEL_0, 0);
            }
            if (now - ledPatternStart > 50) {
                ledPatternStep++;
                ledPatternStart = now;
            }
            if (ledPatternStep > 5) {
                currentLedPattern = LED_OFF;
            }
            break;
        case LED_FAST_BLINK:
            ledcWrite(LEDC_CHANNEL_0, (now / 100) % 2 ? 8191 : 0);
            break;
        case LED_SOLID_ON:
            ledcWrite(LEDC_CHANNEL_0, 8191);
            break;
        case LED_FADE:
            // Smooth 2-second fade cycle - continuous fade from bottom to top and back
            if (now - lastFadeUpdate > 20) { // Update every 20ms for smooth fade
                static bool started = false;
                
                if (!started) {
                    fadeValue = 0; // Start from bottom (off)
                    fadeDirection = 1; // Start fading up
                    started = true;
                }
                
                fadeValue += fadeDirection * 20; // Smaller increment for smooth fade
                
                if (fadeValue >= 8191) {
                    fadeValue = 8191;
                    fadeDirection = -1; // Start fading down
                } else if (fadeValue <= 0) {
                    fadeValue = 0;
                    fadeDirection = 1; // Start fading up
                }
                
                ledcWrite(LEDC_CHANNEL_0, fadeValue);
                lastFadeUpdate = now;
            }
            break;
        case LED_OFF:
        default:
            ledcWrite(LEDC_CHANNEL_0, 0);
            break;
    }
}