#include <Arduino.h>
#include <logger.h>
#include "pin_map.h"
#include "sim800.h"

SIM800 gsm(Serial1, GSM_TX_PIN, GSM_RX_PIN, GSM_POWER_PIN, GSM_PWRKEY_PIN, GSM_RESET_PIN, GSM_DTR_PIN);

void setup() {
    Serial.begin(115200);
    gsm.powerOn();
    delay(3000);
}

void loop() {
    Serial1.print("AT\r\n");
    while(Serial1.available()) {
        Serial.println(Serial1.readString());
    }
    // gsm.writeMessage("AT");
    // delay(500);
    // printf(gsm.readMessage());
    
    delay(500);
}