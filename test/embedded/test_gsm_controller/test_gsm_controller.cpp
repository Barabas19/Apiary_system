#include <unity.h>
#include <Arduino.h>

#include "pin_map.h"
#include "gsm_controller.h"

GsmController gsm(Serial1, 
    GSM_TX_PIN, GSM_RX_PIN, GSM_POWER_PIN, GSM_PWRKEY_PIN, GSM_RESET_PIN, GSM_DTR_PIN, "internet.t-mobile.cz");

void test_power_on_off() {
    TEST_ASSERT(gsm.powerOffModem());
    TEST_ASSERT(gsm.powerOnModem());
}

void test_gsm_controller() {
    gsm.powerOnModem();
    RUN_TEST(test_power_on_off);
}

void setup() {
    UNITY_BEGIN();
    test_gsm_controller();
}

void loop() {
    UNITY_END();
}
