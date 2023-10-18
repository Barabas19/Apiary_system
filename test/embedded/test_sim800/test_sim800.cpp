#include <unity.h>
#include <string.h>

#include "sim800.h"
#include "pin_map.h"

SIM800 gsm(Serial1, GSM_TX_PIN, GSM_RX_PIN, GSM_POWER_PIN, GSM_PWRKEY_PIN, GSM_RESET_PIN, GSM_DTR_PIN);

void test_sim800_power_on_off() {
    for(int i = 0; i < 5; i++) {
        gsm.powerOff();
        delay(100);
        gsm.powerOn();
        delay(100);
    }

    TEST_ASSERT(true);
}

void test_sim800_read_write() {
    char *rcvMsg;
    gsm.powerOn();
    for(int i = 0; i < 10; i++) {
        gsm.writeMessage("AT");
        while(gsm.messageAvailable()) {
            rcvMsg = gsm.readMessage();
        }
    }
    
    printf(rcvMsg);
    TEST_ASSERT_EQUAL_STRING("OK", rcvMsg);
}

void test_sim800_memory_leak() {
    char *rcvMsg;
    gsm.powerOn();
    for(int i = 0; i < 10; i++) {
        gsm.writeMessage("AT");
        while(gsm.messageAvailable()) {
            rcvMsg = gsm.readMessage();
        }

        if(strcmp(rcvMsg, "OK") == 0) {
            break;
        }
    }

    size_t initialFreeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    int samples = 50;
    int responces = 0;
    for(int i = 0; i < samples; i++) {
        gsm.writeMessage("AT");
        while(gsm.messageAvailable()) {
            rcvMsg = gsm.readMessage();
        }

        if(strcmp(rcvMsg, "OK") == 0) {
            responces++;
        }
    }

    TEST_ASSERT_EQUAL(samples, responces);

    size_t finalFreeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    TEST_ASSERT_EQUAL(initialFreeSize, finalFreeSize);
}


void test_sim800() {
    gsm.powerOn();
    RUN_TEST(test_sim800_power_on_off);
    RUN_TEST(test_sim800_read_write);
    RUN_TEST(test_sim800_memory_leak);
}

void setup() {
    UNITY_BEGIN();
    test_sim800();
}

void loop() {
    UNITY_END();
}