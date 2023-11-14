#ifndef WEB_APP_CONNECTOR_H_
#define WEB_APP_CONNECTOR_H_

#include <Arduino.h>

#include "gsm_controller.h"
#include "config_handler.h"

#define POST_PHP_FILE "req/store-sensor-data.php"

class WebAppConnector {
public:
    WebAppConnector(GsmController &gsm, const char *webAppServer);

    /** Initialize, update system time from GSM network
     * @returns - true, if successfully initialized
    */
    bool init();

    /** Upload collected sensor values and update config (if any change received)
     * @dataJsonPtr - pointer to sensor values in json format
     * @configJsonPtr - pointer to the obtained config in json format
     * @returns - true, if successfully initialized
    */
    bool uploadData(const char *dataJsonPtr, const char *configJsonPtr);

private:
    GsmController &gsm;
    bool initialized;
    char postPayload[128];
    const char *webAppServer;

    bool transformJsonToPostData(const char *jsonPptr, char* postDataPtr);
    
};

#endif // WEB_APP_CONNECTOR_H_