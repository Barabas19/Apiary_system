#include <Arduino.h>
#include <logger.h>
#include <ArduinoJson.h>
#include <string>

#include "gsm_controller.h"
#include "pin_map.h"
#include "config_handler.h"
#include "value_set.h"

using namespace Esp32Logger;

GsmController gsm(Serial1, GSM_TX_PIN, GSM_RX_PIN, GSM_POWER_PIN, GSM_PWRKEY_PIN, GSM_RESET_PIN, GSM_DTR_PIN);


void setup() {
    delay(2000);
    ValueSet vs;
    time_t now;
    time(&now);
    vs.setTimestamp(now);
    vs.addValue("test", 100.0);
    auto poststr = vs.getPostString();
    if(poststr) {
        LOG_I("%s", poststr);
    } else {
        LOG_I("nullptr");
    }
}

void loop() {
}