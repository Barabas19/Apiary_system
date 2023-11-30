#include <unity.h>
#include <string.h>

#include "value_set.h"
#include "sensor_data_struct.h"

void test_timestamp() {
    const time_t tm = 10;
    ValueSet vs;
    vs.setTimestamp(tm);
    TEST_ASSERT_EQUAL(tm, vs.getTimestamp());
}

void test_value() {
    SensorData srcData;
    srcData.name = "test";
    srcData.value = 10.0;
    srcData.battery = 99;
    srcData.rssi = -150;
    ValueSet vs;
    vs.addSensorData(srcData);

    SensorData dstData;
    dstData.name = srcData.name;
    TEST_ASSERT(vs.getSensorData(dstData));
    TEST_ASSERT_EQUAL(srcData.value, dstData.value);
    TEST_ASSERT_EQUAL(srcData.battery, dstData.battery);
    TEST_ASSERT_EQUAL(srcData.rssi, dstData.rssi);
}

void test_from_to_string() {
    const time_t tm = 10;
    const char *name = "test";
    const float value = 20.0;
    const uint8_t battery = 99;
    const int rssi = -150;
    char srcStr[128];
    const char *jsonFormat = "{\"timestamp\":%d,\"sensors\":[{\"name\":\"%s\",\"value\":%.2f,\"battery\":%u,\"rssi\":%d}]}";
    sprintf(srcStr, jsonFormat, tm, name, value, battery, rssi);
    ValueSet vs;
    TEST_ASSERT(ValueSet::fromJsonString(srcStr, vs));
    auto dstStr = vs.toJsonString();
    TEST_ASSERT_EQUAL_STRING(srcStr, dstStr.c_str());
}

void test_value_set() {
    RUN_TEST(test_timestamp);
    RUN_TEST(test_value);
    RUN_TEST(test_from_to_string);
}

void setup() {
    UNITY_BEGIN();
    test_value_set();
}

void loop() {
    UNITY_END();
}
