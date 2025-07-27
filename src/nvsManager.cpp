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

#include "nvsManager.h"
#include "config.h"
#include "globals.h"
#include "debug.h"
#include "utils.h"
#include <Preferences.h>

void saveMidiMapToNVS() {
    Preferences nvs;
    if (nvs.begin("midi_map", false)) {
        nvs.putBytes("map", midiChannelMap, MAX_AMPSWITCHS);
        nvs.putInt("version", STORAGE_VERSION);
        nvs.end();
        log(LOG_INFO, "MIDI channel map saved to NVS");
    } else {
        log(LOG_ERROR, "Failed to save MIDI channel map to NVS");
    }
}

void loadMidiMapFromNVS() {
    Preferences nvs;
    if (nvs.begin("midi_map", true)) {
        if (nvs.getInt("version", 0) != STORAGE_VERSION) {
            // Version mismatch: reset to defaults and save
            for (int i = 0; i < MAX_AMPSWITCHS; i++) midiChannelMap[i] = i;
            nvs.end();
            nvs.begin("midi_map", false);
            nvs.putBytes("map", midiChannelMap, MAX_AMPSWITCHS);
            nvs.putInt("version", STORAGE_VERSION);
            nvs.end();
            log(LOG_WARN, "MIDI map NVS version mismatch, resetting to defaults");
        } else if (nvs.getBytesLength("map") == MAX_AMPSWITCHS) {
            nvs.getBytes("map", midiChannelMap, MAX_AMPSWITCHS);
            nvs.end();
            log(LOG_INFO, "MIDI channel map loaded from NVS");
        } else {
            nvs.end();
        }
    }
}

void saveMidiChannelToNVS() {
    Preferences nvs;
    if (nvs.begin("midi_channel", false)) {
        nvs.putUChar("channel", currentMidiChannel);
        nvs.putInt("version", STORAGE_VERSION);
        nvs.end();
        log(LOG_INFO, "MIDI channel " + String(currentMidiChannel) + " saved to NVS");
    } else {
        log(LOG_ERROR, "Failed to save MIDI channel to NVS");
    }
}

void loadMidiChannelFromNVS() {
    Preferences nvs;
    if (nvs.begin("midi_channel", true)) {
        if (nvs.getInt("version", 0) != STORAGE_VERSION) {
            // Version mismatch: reset to defaults and save
            currentMidiChannel = 1;
            nvs.end();
            nvs.begin("midi_channel", false);
            nvs.putUChar("channel", currentMidiChannel);
            nvs.putInt("version", STORAGE_VERSION);
            nvs.end();
            log(LOG_WARN, "MIDI channel NVS version mismatch, resetting to default");
        } else if (nvs.isKey("channel")) {
            currentMidiChannel = nvs.getUChar("channel", 1);
            nvs.end();
            log(LOG_INFO, "MIDI channel " + String(currentMidiChannel) + " loaded from NVS");
        } else {
            nvs.end();
        }
    }
}

// Log level NVS functions
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

// ESP-NOW pairing NVS functions
void clearPairingNVS() {
    Preferences nvs;
    if (nvs.begin(NVS_NAMESPACE, false)) {
        nvs.clear();
        nvs.putInt("version", STORAGE_VERSION);
        nvs.end();
        log(LOG_INFO, "Pairing info cleared from NVS");
    } else {
        log(LOG_ERROR, "Failed to clear pairing NVS");
    }
}

void saveServerToNVS(const uint8_t* mac, uint8_t channel) {
    Preferences nvs;
    log(LOG_DEBUG, "Saving server info to NVS...");
    if (nvs.begin(NVS_NAMESPACE, false)) {
        nvs.putBytes("server_mac", mac, 6);
        nvs.putUChar("channel", channel);
        nvs.putInt("version", STORAGE_VERSION);
        log(LOG_INFO, "Server info saved to NVS:");
        printMAC(mac, LOG_INFO);
        log(LOG_INFO, "Channel: " + String(channel));
        nvs.end();
    } else {
        log(LOG_ERROR, "Failed to open NVS for writing!");
    }
}

bool loadServerFromNVS(uint8_t* mac, uint8_t* channel) {
    Preferences nvs;
    log(LOG_DEBUG, "Loading server info from NVS...");
    bool success = false;
    if (nvs.begin(NVS_NAMESPACE, true)) {
        if (nvs.getInt("version", 0) != STORAGE_VERSION) {
            log(LOG_WARN, "Incorrect NVS version, resetting NVS");
            nvs.end();
            clearPairingNVS();
            return success;
        }
        if (nvs.getBytesLength("server_mac") == 6) {
            nvs.getBytes("server_mac", mac, 6);
            *channel = nvs.getUChar("channel", 1);
            log(LOG_INFO, "Server info loaded from NVS:");
            printMAC(mac, LOG_INFO);
            log(LOG_INFO, "Channel: " + String(*channel));
            success = true;
        } else {
            log(LOG_DEBUG, "No server MAC found in NVS");
        }
        nvs.end();
    } else {
        log(LOG_ERROR, "Failed to open NVS for reading!");
    }
    return success;
}
