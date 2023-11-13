#include <unity.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "project_config.h"
#include "pin_map.h"
#include "gsm_controller.h"
#include "web_app_connector.h"

GsmController gsm(Serial1, GSM_TX_PIN, GSM_RX_PIN, GSM_POWER_PIN, GSM_PWRKEY_PIN, GSM_RESET_PIN, GSM_DTR_PIN);
WebAppConnector webConnector(gsm, CONF_WEB_SERVER);

void test_init() {
    TEST_ASSERT(webConnector.init());
}

void test_upload_data() {
    char data[] = "{\"sensor\"=100.0}";
    const char *configJsonPtr;
    TEST_ASSERT(webConnector.uploadData(data, configJsonPtr));

    if(configJsonPtr) {
        DynamicJsonDocument doc(strlen(configJsonPtr) * 2);
        TEST_ASSERT(deserializeJson(doc, configJsonPtr) == DeserializationError::Ok);
    }
}

void test_web_app_connector() {
    RUN_TEST(test_init);
    RUN_TEST(test_upload_data);
}

void setup() {
    UNITY_BEGIN();
    test_web_app_connector();
}

void loop() {
    UNITY_END();
}
