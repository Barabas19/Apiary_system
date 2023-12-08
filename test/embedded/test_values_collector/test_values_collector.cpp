#include <unity.h>
#include <Arduino.h>

#include <values_collector.h>

TaskHandle_t taskHandler;

void values_collecting_task(void *params) {
    struct tm dt;
    ValuesCollector::run(dt);
}

void test_collecing_window() {
    xTaskCreate(values_collecting_task, "values_collecting", 10000, NULL, 1, &taskHandler);
    time_t now = time(NULL);
    time_t end = now + 60;
    while(now < end) {
        TEST_ASSERT(!ValuesCollector::isFinished());
        delay(9000);
    }

    delay(3000);
    TEST_ASSERT(ValuesCollector::isFinished());
}

void test_calculate_sleep_time() {
    time_t requiredSleep = 60;
    time_t now = time(NULL);
    time_t next = now + requiredSleep;
    struct tm *nextWindowdt = localtime(&now);
    time_t calculatedSleep = ValuesCollector::calculateSleepTime(*nextWindowdt);
    TEST_ASSERT_EQUAL(requiredSleep, calculatedSleep);
}
