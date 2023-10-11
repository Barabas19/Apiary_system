#include <Arduino.h>
#include <logger.h>

void setup() {}

void loop() {
    static uint32_t cycle = 0;
    Esp32Logger::getFileName("/home/dd/file.txt");
    
    LOG_I("Test message nr %u", ++cycle);
    sleep(1);
}