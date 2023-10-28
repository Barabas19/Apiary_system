#include <unity.h>
#include <Arduino.h>

#include "pin_map.h"
#include "gsm_controller.h"

GsmController gsm(Serial1, 
    GSM_TX_PIN, GSM_RX_PIN, GSM_POWER_PIN, GSM_PWRKEY_PIN, GSM_RESET_PIN, GSM_DTR_PIN);

void test_power_on_off() {
    TEST_ASSERT(gsm.powerOffModem());
    TEST_ASSERT(gsm.powerOnModem());
}

void test_init_deinit() {
    TEST_ASSERT(gsm.init());
    TEST_ASSERT(gsm.deinit());
}

void test_gsm_controller() {
    gsm.powerOnModem();
    RUN_TEST(test_power_on_off);
    RUN_TEST(test_init_deinit);
}

void setup() {
    UNITY_BEGIN();
    test_gsm_controller();
}

void loop() {
    UNITY_END();
}
