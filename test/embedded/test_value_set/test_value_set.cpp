#include <unity.h>
#include <string.h>

#include "value_set.h"

void test_timestamp() {
    const time_t tm = 10;
    ValueSet vs;
    vs.setTimestamp(tm);
    TEST_ASSERT_EQUAL(tm, vs.getTimestamp());
}

void test_value() {
    const char *name = "test";
    const float value = 10.0;
    ValueSet vs;
    vs.addValue(name, value);
    TEST_ASSERT_EQUAL(value, vs.getValue(name));
}

void test_from_to_string() {
    const time_t tm = 10;
    const char *name = "test";
    const float value = 20.0;
    char buffer[32];
    sprintf(buffer, "timestamp=%d&%s=%.2f", tm, name, value);
    ValueSet vs = ValueSet::fromString(buffer);
    auto createdStr = vs.toString();
    TEST_ASSERT(buffer == createdStr);
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
