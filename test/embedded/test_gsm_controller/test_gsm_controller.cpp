#include <unity.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "pin_map.h"
#include "gsm_controller.h"

const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

GsmController gsm(Serial1, 
    GSM_TX_PIN, GSM_RX_PIN, GSM_POWER_PIN, GSM_PWRKEY_PIN, GSM_RESET_PIN, GSM_DTR_PIN);

void test_power_on_off() {
    TEST_ASSERT(gsm.powerOffModem());
    TEST_ASSERT(gsm.powerOnModem());
}

void test_init() {
    TEST_ASSERT(gsm.init());
}

void test_get_date_time_location() {
    struct tm dt;
    double longitude = 0, latitude = 0;
    TEST_ASSERT(gsm.getTimeDateLocation(dt, &longitude, &latitude));
    char buff[64];
    strcpy(buff, __DATE__);
    char *token = strtok(buff, " ");
    TEST_ASSERT(strcmp(months[dt.tm_mon], token));
    TEST_ASSERT_EQUAL(dt.tm_mday, atoi(strtok(NULL, " ")));
    TEST_ASSERT_EQUAL(dt.tm_year, atoi(strtok(NULL, " ")));
}

void test_get_local_date_time() {
    struct tm dt;
    TEST_ASSERT(gsm.getLocalDateTime(dt));
    char buff[64];
    strcpy(buff, __DATE__);
    char *token = strtok(buff, " ");
    TEST_ASSERT(strcmp(months[dt.tm_mon], token));
    TEST_ASSERT_EQUAL(dt.tm_mday, atoi(strtok(NULL, " ")));
    TEST_ASSERT_EQUAL(dt.tm_year, atoi(strtok(NULL, " ")));
}

void test_send_http_get_request() {
    char buff[128];
    char url[128];
    const char *key = "Hello";
    const char *value = "Apiary";
    sprintf(url, "%s/test/http-test.php?%s=%s", CONF_WEB_SERVER, key, value);
    TEST_ASSERT(gsm.sendHttpGetReq(url, buff, sizeof(buff)));
    DynamicJsonDocument doc(128);
    deserializeJson(doc, buff);
    TEST_ASSERT(doc["method"] == "GET");
    TEST_ASSERT(doc[key] == value);
}

void test_send_http_post_request() {
    char buff[128];
    char url[128];
    char data[128];
    const char *key = "Hello";
    const char *value = "Apiary";
    sprintf(url, "%s/test/http-test.php", CONF_WEB_SERVER);
    sprintf(data, "%s=%s", key, value);
    TEST_ASSERT(gsm.sendHttpPostReq(url, data, buff, sizeof(buff)));
    DynamicJsonDocument doc(128);
    deserializeJson(doc, buff);
    TEST_ASSERT(doc["method"] == "POST");
    TEST_ASSERT(doc[key] == value);
}

void test_deinit() {
    TEST_ASSERT(gsm.deinit());
}

void test_gsm_controller() {
    gsm.powerOnModem();
    RUN_TEST(test_power_on_off);
    RUN_TEST(test_init);
    RUN_TEST(test_get_date_time_location);
    RUN_TEST(test_get_local_date_time);
    RUN_TEST(test_send_http_get_request);
    RUN_TEST(test_send_http_post_request);
    RUN_TEST(test_deinit);
}

void setup() {
    UNITY_BEGIN();
    test_gsm_controller();
}

void loop() {
    UNITY_END();
}
