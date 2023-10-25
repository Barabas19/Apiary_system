#include <unity.h>
#include <Arduino.h>

#include "config_handler.h"

void test_get_burst() {
    uint16_t burst;
    TEST_ASSERT(Config::getBurst(&burst));
    TEST_ASSERT(burst == DEFAULT_BURST);
}

void test_set_burst() {
    uint16_t burst = DEFAULT_BURST / 2;
    TEST_ASSERT(Config::setBurst(burst));
    uint16_t loadedBurst;
    Config::getBurst(&loadedBurst);
    TEST_ASSERT_EQUAL(burst, loadedBurst);
    Config::setBurst(DEFAULT_BURST);
    Config::getBurst(&loadedBurst);
    TEST_ASSERT_EQUAL(DEFAULT_BURST, loadedBurst);
}

void test_settings() {
    RUN_TEST(test_get_burst);
    RUN_TEST(test_set_burst);
}

void setup() {
    UNITY_BEGIN();
    test_settings();
}

void loop() {
    UNITY_END();
}