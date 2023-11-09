#ifndef WEB_APP_CONNECTOR_H_
#define WEB_APP_CONNECTOR_H_

#include <Arduino.h>

#include "gsm_controller.h"
#include "config_handler.h"

#define POST_PHP_FILE "req/store-sensor-data.php"

class WebAppConnector {
public:
    WebAppConnector(GsmController &gsm, Config &config);

    /** Initialize, update system time from GSM network
     * @returns - true, if successfully initialized
    */
    bool init();

    bool uploadData(const char *dataJsonPtr);
    bool updateConfig(const char *configJsonPtr);

private:
    GsmController &gsm;
    Config &config;
    bool initialized;

    bool transformJsonToPostData(const char *jsonPptr, char* postDataPtr);
};

#endif // WEB_APP_CONNECTOR_H_