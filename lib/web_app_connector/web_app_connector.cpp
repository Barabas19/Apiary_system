#include "web_app_connector.h"
#include "logger.h"
#include "config.h"

#include <time.h>
#include <ArduinoJson.h>

WebAppConnector::WebAppConnector(GsmController &gsm, Config &config) :
gsm{gsm},
config{config},
initialized{false} {}

bool WebAppConnector::init()
{
    if(initialized) {
        LOG_I("Already initialized.");
        return true;
    }

    LOG_V("Initialize web connector...");

    // initialize GSM
    if(!gsm.init()) {
        LOG_E("Failed to initialize gsm controller.");
        return false;
    }

    // update system time
    struct tm dt;
    if(!gsm.getLocalDateTime(dt)) {
        LOG_E("Failed to obtain GSM date/time.");
        return false;
    }

    struct timeval tv;
    tv.tv_sec = mktime(&dt);
    tv.tv_usec = 0;
    if(settimeofday(&tv, NULL) < 0) {
        LOG_W("Cannot update system time.");
    }
    
    initialized = true;
    LOG_I("Web connector initialized.");
    return true;
}

bool WebAppConnector::uploadData(const char *dataJsonPtr)
{
    if(!initialized) {
        LOG_E("Failed to upload data - web connector not initialized.");
        return false;
    }

    if(dataJsonPtr == nullptr) {
        LOG_E("Failure - no data provided.");
        return false;
    }

    char *postData = (char *)calloc(strlen(dataJsonPtr), 1);
    bool res = transformJsonToPostData(dataJsonPtr, postData);
    if(!res) {
        LOG_E("Failed to prepare data for uploading.");
    }

    char url[64];
    char payload[64];
    sprintf(url, "%s/%s",CONF_WEB_SERVER, POST_PHP_FILE);
    if(res) {
        res = gsm.sendHttpPostReq(url, postData, payload, sizeof(payload));
        if(!res) {
            LOG_E("Failed to upload data.");
        }
    }

    free(postData);
    return res;
}

bool WebAppConnector::transformJsonToPostData(const char *jsonPptr, char *postDataPtr)
{
    if(!jsonPptr || !postDataPtr) {
        return false;
    }

    DynamicJsonDocument doc(strlen(jsonPptr) * 2);
    if(deserializeJson(doc, jsonPptr) != DeserializationError::Ok) {
        LOG_E("Failed to deserialize data json.");
        return false;
    }

    if(doc.nesting() > 1) {
        LOG_E("Failure - only json with depth 1 can be used.");
        return false;
    }

    bool firstValue = true;
    JsonObject root = doc.as<JsonObject>();
    for(JsonPair kv : root) {
        if(firstValue) {
            firstValue = false;
        } else {
            strcat(postDataPtr, "&");
        }

        sprintf(postDataPtr, "%s%s=%.2f", postDataPtr, kv.key().c_str(), kv.value().as<float>());
    }

    return true;
}
