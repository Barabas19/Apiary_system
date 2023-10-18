#include <Arduino.h>
#include <logger.h>
#include "pin_map.h"
#include "sim800.h"

using namespace Esp32Logger;

SIM800 gsm(Serial1, GSM_TX_PIN, GSM_RX_PIN, GSM_POWER_PIN, GSM_PWRKEY_PIN, GSM_RESET_PIN, GSM_DTR_PIN);
char *buff;

void setup() {
    gsm.powerOn();
    delay(3000);
}

void loop() {
    LOG_I("----------");
    gsm.writeMessage("AT");
    delay(1000);
    
    while(gsm.messageAvailable()) {
        auto message = gsm.readMessage();
        if(message) {
            LOG_I("%s", message);
        }
    }
    
    delay(1000);
}