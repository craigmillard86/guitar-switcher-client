#include "utils.h"
#include <espnow-pairing.h>
#include <commandHandler.h>

void log(LogLevel level, const String& msg) {
    if (level <= currentLogLevel) {
        Serial.println(msg);
    }
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
void checkSerialCommands() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        // --- Existing commands ---
        if (cmd.equalsIgnoreCase("ota")) {
            serialOtaTrigger = true;

        } else if (cmd.equalsIgnoreCase("status")) {
            extern uint8_t clientMacAddress[6];
            printMAC(clientMacAddress, LOG_INFO);
            Serial.print("Current amp channel: ");
            Serial.println(currentAmpChannel);

        } else if (cmd.equalsIgnoreCase("reset")) {
            log(LOG_ERROR, "[Client] Restarting ESP...");
            delay(1000);
            ESP.restart();

        } else if (cmd.equalsIgnoreCase("pair")) {
            clearPairingNVS();
            pairingStatus = PAIR_REQUEST;
            log(LOG_ERROR, "[Client] Re-pairing requested!");

        } else if (cmd.startsWith("setlog")) {
            int level = cmd.substring(6).toInt();
            extern LogLevel currentLogLevel;
            currentLogLevel = (LogLevel)level;
            log(LOG_ERROR, "[Client] Log level set to: " + String(level));

        // --- Amp channel/relay test commands ---
        } else if (cmd.equalsIgnoreCase("help")) {
            Serial.println(F("======== SERIAL COMMANDS ========"));
            Serial.println(F("ota         : Trigger OTA (firmware update) mode."));
            Serial.println(F("status      : Print current device and amp status."));
            Serial.println(F("reset       : Reboot the device."));
            Serial.println(F("pair        : Clear pairing and request re-pairing."));
            Serial.println(F("setlogN     : Set log level (N=0-OFF,1=ERR,2=WARN,3=INFO,4=DEBUG)."));
            Serial.println(F("help        : Show this help message."));
            Serial.println();
            Serial.println(F("==== Amp Channel Control Commands ===="));
            Serial.println(F("1-4         : Directly switch to amp channel 1-4."));
            Serial.println(F("b1-b4       : Simulate physical button press 1-4."));
            Serial.println(F("off         : Turn all amp channels off."));
            Serial.println(F("status      : Show current amp channel and MAC."));
            Serial.println();
            Serial.println(F("Examples:"));
            Serial.println(F("  setlog3   : Show info and above logs."));
            Serial.println(F("  2         : Switch to channel 2."));
            Serial.println(F("  b3        : Simulate button 3 press."));
            Serial.println(F("  ota       : Trigger OTA update."));
            Serial.println();
            Serial.println(F("======================================"));        
        } else if (cmd.equalsIgnoreCase("off")) {
            setAmpChannel(0); // Implement "all channels off" in your setAmpChannel()
            Serial.println("All channels off.");

        } else if (cmd.toInt() > 0 && cmd.toInt() <= MAX_AMPSWITCHS) {
            uint8_t ch = cmd.toInt();
            setAmpChannel(ch);
            Serial.print("Channel set to ");
            Serial.println(ch);

        } else if (cmd.length() == 2 && cmd[0] == 'b') {
            int btn = cmd[1] - '0';
            if (btn >= 1 && btn <= MAX_AMPSWITCHS) {
                Serial.print("Simulating button ");
                Serial.println(btn);
                setAmpChannel(btn); // Or call a button handler if you want to simulate debounce etc.
            } else {
                Serial.println("Invalid button number");
            }

        } else {
            log(LOG_WARN, "[Client] Unknown command: " + cmd);
            Serial.println("Unknown command. Type 'help' for command list.");
        }
    }
}