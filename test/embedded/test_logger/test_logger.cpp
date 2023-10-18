#include <unity.h>
#include <esp_heap_caps.h>
#include <logger.h>

#define LOG_LEVEL 5

void setUp(void) {}

void tearDown(void) {}

void test_heap_free_size() {
    size_t heapFreeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(200000, heapFreeSize, "Not enough memory");
}

void test_memory_leak_short_messages() {
    LOG_I("Initial message");
    size_t initialFreeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    int samples = 50;
    for(int i = 0; i < samples; i++) {
        LOG_E("Short message nr %d\n", i);
        LOG_W("Short message nr %d\n", i);
        LOG_I("Short message nr %d\n", i);
        LOG_D("Short message nr %d\n", i);
        LOG_V("Short message nr %d\n", i);
    }
    size_t finalFreeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    TEST_ASSERT_EQUAL(initialFreeSize, finalFreeSize);
}

void test_memory_leak_long_messages() {
    char buff[DEFAULT_MESSAGE_LENGTH];
    for(int i = 0; i < DEFAULT_MESSAGE_LENGTH - 1; i++) {
        buff[i] = ' ';
    }
    buff[DEFAULT_MESSAGE_LENGTH - 1] = 0;
    LOG_I("Initial message");
    size_t initialFreeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    int samples = 50;
    for(int i = 0; i < samples; i++) {
        LOG_E("Long message nr %d: %s\n", i, buff);
        LOG_W("Long message nr %d: %s\n", i, buff);
        LOG_I("Long message nr %d: %s\n", i, buff);
        LOG_D("Long message nr %d: %s\n", i, buff);
        LOG_V("Long message nr %d: %s\n", i, buff);
    }
    size_t finalFreeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    TEST_ASSERT_EQUAL(initialFreeSize, finalFreeSize);

}

void test_get_file_name_from_path() {
    const char *pathUnix = "/home/test/file.xml";
    const char *pathWindows = "C:\\system\\file.zip";
    TEST_ASSERT_EQUAL_STRING("file.xml", Esp32Logger::getFileName(pathUnix));
    TEST_ASSERT_EQUAL_STRING("file.zip", Esp32Logger::getFileName(pathWindows));
}

void test_logger() {
    RUN_TEST(test_heap_free_size);
    RUN_TEST(test_memory_leak_short_messages);
    RUN_TEST(test_memory_leak_long_messages);
    RUN_TEST(test_get_file_name_from_path);
}

void setup() {
    UNITY_BEGIN();
    test_logger();
}

void loop() {
    UNITY_END();
}