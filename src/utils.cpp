#include "utils.h"
#include <espnow-pairing.h>
#include <commandHandler.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include "debug.h"
#include <Preferences.h>

extern unsigned long lastMemoryCheck;

// Global variables for memory tracking
uint32_t minFreeHeap = UINT32_MAX;
// unsigned long lastMemoryCheck = 0; // Removed to avoid multiple definition

// NVS functions for log level persistence
void saveLogLevelToNVS(LogLevel level) {
    Preferences nvs;
    if (nvs.begin("logging", false)) {
        nvs.putUChar("log_level", (uint8_t)level);
        nvs.putInt("version", STORAGE_VERSION);
        nvs.end();
        log(LOG_DEBUG, "Log level saved to NVS: " + String((uint8_t)level) + " (version " + String(STORAGE_VERSION) + ")");
    } else {
        log(LOG_ERROR, "Failed to save log level to NVS");
    }
}

LogLevel loadLogLevelFromNVS() {
    Preferences nvs;
    LogLevel level = LOG_INFO; // Default level
    if (nvs.begin("logging", true)) {
        // Check storage version
        if (nvs.getInt("version", 0) != STORAGE_VERSION) {
            log(LOG_WARN, "Incorrect logging NVS version, using default log level");
            nvs.end();
            return level;
        }
        
        uint8_t savedLevel = nvs.getUChar("log_level", LOG_INFO);
        level = (LogLevel)savedLevel;
        nvs.end();
        log(LOG_DEBUG, "Log level loaded from NVS: " + String((uint8_t)level) + " (version " + String(STORAGE_VERSION) + ")");
    } else {
        log(LOG_WARN, "Failed to load log level from NVS, using default");
    }
    return level;
}

void clearLogLevelNVS() {
    Preferences nvs;
    if (nvs.begin("logging", false)) {
        nvs.clear();
        nvs.putInt("version", STORAGE_VERSION);
        nvs.end();
        log(LOG_INFO, "Log level NVS cleared");
    } else {
        log(LOG_ERROR, "Failed to clear log level NVS");
    }
}

// Enhanced logging with timestamps and proper log levels
void log(LogLevel level, const String& msg) {
    if (level <= currentLogLevel) {
        String timestamp = getUptimeString();
        String levelStr = getLogLevelString(level);
        Serial.printf("[%s][%s] %s\n", timestamp.c_str(), levelStr.c_str(), msg.c_str());
    }
}

void logf(LogLevel level, const char* format, ...) {
    if (level <= currentLogLevel) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        log(level, String(buffer));
    }
}

void logWithTimestamp(LogLevel level, const String& msg) {
    log(level, msg);
}

void printMAC(const uint8_t *mac, LogLevel level) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    log(level, String(macStr));
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
    log(LOG_INFO, "Firmware Version: " + String(FIRMWARE_VERSION));
    log(LOG_INFO, "Board ID: " + String(BOARD_ID));
    log(LOG_INFO, "Uptime: " + getUptimeString());
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
    
    log(LOG_INFO, "Memory - Free: " + String(freeHeap) + "B, Used: " + String(usedHeap) + "B (" + String(usagePercent, 1) + "%)");
    log(LOG_INFO, "Min Free Heap: " + String(minFreeHeap) + "B");
}

void printNetworkStatus() {
    log(LOG_INFO, "WiFi Mode: " + String(WiFi.getMode()));
    log(LOG_INFO, "Current Channel: " + String(currentChannel));
    log(LOG_INFO, "Client MAC: ");
    printMAC(clientMacAddress, LOG_INFO);
    log(LOG_INFO, "Server MAC: ");
    printMAC(serverAddress, LOG_INFO);
}

void printAmpChannelStatus() {
    log(LOG_INFO, "Current Amp Channel: " + String(currentAmpChannel));
    log(LOG_INFO, "Channel Pins: " + String(ampSwitchPins[0]) + "," + String(ampSwitchPins[1]) + "," + 
                   String(ampSwitchPins[2]) + "," + String(ampSwitchPins[3]));
    log(LOG_INFO, "Button Pins: " + String(ampButtonPins[0]) + "," + String(ampButtonPins[1]) + "," + 
                   String(ampButtonPins[2]) + "," + String(ampButtonPins[3]));
}

void printPairingStatus() {
    log(LOG_INFO, "Pairing Status: " + getPairingStatusString(pairingStatus));
}

// Enhanced serial command handling
void checkSerialCommands() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        handleSerialCommand(cmd);
    }
}

void handleSerialCommand(const String& cmd) {
    if (cmd.isEmpty()) return;

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
    } else if (cmd.equalsIgnoreCase("uptime")) {
        log(LOG_INFO, "Uptime: " + getUptimeString());
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
            log(LOG_INFO, "Log level set to: " + getLogLevelString(currentLogLevel));
        } else {
            log(LOG_WARN, "Invalid log level. Use 0-4 (0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG)");
        }
    } else if (cmd.equalsIgnoreCase("testled")) {
        log(LOG_INFO, "Testing status LED...");
        blinkLED(STATUS_LED_PIN, 3, 200);
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
        log(LOG_INFO, "Amp channel set to " + String(ch));
    } else if (cmd.length() == 2 && cmd[0] == 'b') {
        int btn = cmd[1] - '0';
        if (btn >= 1 && btn <= MAX_AMPSWITCHS) {
            log(LOG_INFO, "Simulating button " + String(btn) + " press");
            setAmpChannel(btn);
        } else {
            log(LOG_WARN, "Invalid button number. Use b1-b" + String(MAX_AMPSWITCHS));
        }
    } else if (cmd.equalsIgnoreCase("midi")) {
        log(LOG_INFO, "MIDI Status:");
        log(LOG_INFO, "  MIDI Channel: OMNI (listening to all)");
        log(LOG_INFO, "  MIDI Thru: Enabled");
        log(LOG_INFO, "  MIDI Pins - RX: " + String(MIDI_RX_PIN) + ", TX: " + String(MIDI_TX_PIN));
    } else if (cmd.equalsIgnoreCase("version")) {
        log(LOG_INFO, "Firmware Version: " + String(FIRMWARE_VERSION));
        log(LOG_INFO, "Storage Version: " + String(STORAGE_VERSION));
    } else if (cmd.equalsIgnoreCase("buttons")) {
        enableButtonChecking = !enableButtonChecking;
        log(LOG_INFO, "Button checking " + String(enableButtonChecking ? "enabled" : "disabled"));
    } else if (cmd.equalsIgnoreCase("loglevel")) {
        log(LOG_INFO, "Current log level: " + getLogLevelString(currentLogLevel) + " (" + String((uint8_t)currentLogLevel) + ")");
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
    } else {
        log(LOG_WARN, "Unknown command: '" + cmd + "'");
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
    Serial.println(F("  uptime      : Show system uptime"));
    Serial.println(F("  version     : Show firmware version"));
    Serial.println(F("  midi        : Show MIDI configuration"));
    Serial.println(F("  buttons     : Toggle button checking on/off"));
    Serial.println(F("  loglevel    : Show current log level"));
    Serial.println(F("  clearlog    : Clear saved log level (reset to default)"));
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

// Utility functions
String getLogLevelString(LogLevel level) {
    switch (level) {
        case LOG_NONE: return "NONE";
        case LOG_ERROR: return "ERROR";
        case LOG_WARN: return "WARN";
        case LOG_INFO: return "INFO";
        case LOG_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

String getPairingStatusString(PairingStatus status) {
    switch (status) {
        case NOT_PAIRED: return "NOT_PAIRED";
        case PAIR_REQUEST: return "PAIR_REQUEST";
        case PAIR_REQUESTED: return "PAIR_REQUESTED";
        case PAIR_PAIRED: return "PAIR_PAIRED";
        default: return "UNKNOWN";
    }
}

String getUptimeString() {
    unsigned long uptime = millis();
    unsigned long seconds = uptime / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    char buffer[32];
    if (days > 0) {
        snprintf(buffer, sizeof(buffer), "%lud %02lu:%02lu:%02lu", days, hours % 24, minutes % 60, seconds % 60);
    } else if (hours > 0) {
        snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes % 60, seconds % 60);
    } else {
        snprintf(buffer, sizeof(buffer), "%02lu:%02lu", minutes, seconds % 60);
    }
    return String(buffer);
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