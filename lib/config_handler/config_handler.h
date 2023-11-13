#ifndef CONFIG_HANDLER_H_
#define CONFIG_HANDLER_H_

#include <stdint.h>

#define CONFIG_FILE "/Config.json"
#define DEFAULT_BURST 3600

class Config {
public:
    static bool getBurst(uint16_t *burst);
    static bool setBurst(uint16_t burst);
    static bool updateConfig(const char *configJsonPtr);
    Config() = delete;

private:
    static uint16_t sett_burst;

    static bool initialized;
    static bool init();
    static bool loadConfig();
    static void saveConfig(bool initial = false);
};

#endif // CONFIG_HANDLER_H_