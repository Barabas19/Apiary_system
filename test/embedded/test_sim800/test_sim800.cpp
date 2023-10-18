#include <unity.h>
#include <sim800.h>

void test_sim800_read_write() {
    SIM800 gsm(Serial1, powerPin, pwrKeyPin, rstPin, dtrPin);
    gsm.powerOn();
    delay(1000);
    for(int i = 0; i < 10; i++) {
        gsm.writeMessage("AT");
        printf("AT sent\n");
        delay(1000);
    }
    
    char *rcvMsg = gsm.readMessage();
    // printf(rcvMsg);
    // gsm.powerOff();
    TEST_ASSERT_NOT_EQUAL_MESSAGE(nullptr, rcvMsg, "Nothing read from Serial1");
}

void test_sim800() {
    RUN_TEST(test_sim800_read_write);
}

void setup() {
    UNITY_BEGIN();
    test_sim800();
}

void loop() {
    UNITY_END();
}