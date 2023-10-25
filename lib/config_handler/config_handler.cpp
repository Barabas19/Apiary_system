#include <LittleFS.h>
#include <ArduinoJson.h>

#include "config_handler.h"
#include "logger.h"

uint16_t Config::sett_burst = DEFAULT_BURST;
bool Config::initialized = false;

bool Config::getBurst(uint16_t *burst) {
    if(!initialized && !init()) {
        return false;
    }

    *burst = sett_burst;
    return true;
}

bool Config::setBurst(uint16_t burst) {
    if(!initialized && !init()) {
        return false;
    }

    sett_burst = burst;
    Config::saveConfig();
    return true;
}

bool Config::init() {
    initialized = true;
    if(!LittleFS.begin(true)) {
        LOG_E("Failed to mount LittleFS.");
        initialized = false;
    } else {
        if(LittleFS.exists(CONFIG_FILE)) {
            if(!loadConfig()) {
                if(!LittleFS.remove(CONFIG_FILE)) {
                    LOG_E("Failed to remove '%s'. Verify file system.", CONFIG_FILE);
                    initialized = false;
                } else {
                    saveConfig(true);
                }
            }
        } else {
            saveConfig(true);
        }
    }
    
    return initialized;
}

bool Config::loadConfig() {
    File file = LittleFS.open(CONFIG_FILE, "r");
    auto content = file.readString().c_str();
    file.close();
    DynamicJsonDocument doc(strlen(content));
    deserializeJson(doc, content);

    if(!doc.containsKey("burst")) {
        LOG_E("Failed to load settings.");
        return false;
    }

    sett_burst = doc["burst"];

    LOG_D("Settings loaded successfully.");
    return true;
}

void Config::saveConfig(bool initial) {
    char content[32];
    DynamicJsonDocument doc(sizeof(content));

    doc["burst"] = sett_burst;

    serializeJson(doc, content);
    File file = LittleFS.open(CONFIG_FILE, "w", initial);
    file.print(content);
    file.close();
    LOG_D("Settings saved.");
}
