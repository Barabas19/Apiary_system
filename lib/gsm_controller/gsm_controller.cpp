#include <regex>

#include "gsm_controller.h"
#include "logger.h"
#include "apn_list.h"

GsmController::GsmController(HardwareSerial &modemUart, gpio_num_t txPin, gpio_num_t rxPin, gpio_num_t powerPin, gpio_num_t pwrKeyPin, gpio_num_t rstPin, gpio_num_t dtrPin) :
apn{nullptr},
initialized{false}
{
    modem = new SIM800(modemUart, txPin, rxPin, powerPin, pwrKeyPin, rstPin, dtrPin);
}

bool GsmController::powerOnModem() {
    LOG_V("Power on modem...");
    modem->powerOn();
    bool res = GsmController::pingModem(5);
    if(res) {
        LOG_I("Modem is powered on.");
        return true;
    }

    LOG_E("Failed to power on modem.");
    return false;
}

bool GsmController::powerOffModem() {
    LOG_V("Power off modem...");
    modem->powerOff();
    bool res = !GsmController::pingModem();
    if(res) {
        LOG_I("Modem is powered off.");
        return true;
    }

    LOG_E("Failed to power off modem.");
    return false;
}

bool GsmController::init() {
    const char *TAG = "Initialization";
    const char *respPtr;
    bool tmpRes = false;
    bool localTimestampEnabled = false;
    initialized = false;

    // steps:
    // 1.  AT -> OK - verify modem state
    // 2.  power on
    // 3.  ATE0 -> OK - echo off
    // 4.  wait for "+CFUN: 1"
    // 5.  AT+CLTS? -> +CLTS: 0(1) - get local timestamp mode
    // 6.  wait for "OK"
    // 7.  AT+CLTS=1;&W -> OK - enable local timestamp
    // 8.  wait for unsolicited *PSNWID: "<mcc>", "<mnc>", "<full network name>", <full network name CI>, "<short network name>",<short network name CI>
    // 9.  AT+CFUN=1,1 -> OK  - set full functionality  and reset MT
    // 10.  AT+CSCLK=0 -> OK - disable slow clock
    // 11. AT+CREG? -> +CREG: - get network registration status
    // 12. wait for "OK"
    // 13. AT+CPIN? -> READY(SIM PIN) - get pin status
    // 14. wait for "OK"
    // 15. wait 5s for any unsolicite messages "SMS Ready"/"Call Ready"
    // 16. open bearer
    // 17. finish

    // 1.  AT -> OK - modem is on/off
    LOG_V("%s: verify modem state...", TAG);
    tmpRes = pingModem();

    // 2.  power on
    if(!tmpRes) {
        LOG_V("%s: no response for 'AT' command.", TAG);
        if(!powerOnModem()) {
            LOG_E("%s: power on failed.", TAG);
            return false;
        }
    }

    // 3.  ATE0 -> OK - echo off
    if(!executeAtCmd("ATE0", "OK")) {
        LOG_E("%s: echo off failed.", TAG);
        return false;
    }

    // 4.  wait for "+CFUN: 1"
    if(!waitForMessage("+CFUN: 1", 10000)) {
        LOG_E("%s: no +CFUN received.", TAG);
        return false;
    }

    // 5.  AT+CLTS? -> +CLTS: [0-9] - get local timestamp mode
    respPtr = executeAtCmd("AT+SLTS?", "+CLTS: [0-9]");
    if(!respPtr) {
        LOG_E("%s: failed to get local timestamp mode.", TAG);
        return false;
    }

    localTimestampEnabled = verifyResponse(respPtr, "+CLTS: 1");

    // 6.  wait for "OK"
    if(!waitForMessage("OK")) {
        LOG_E("%s: no OK received after AT+SLTS?.", TAG);
        return false;
    }

    // 7.  AT+CLTS=1;&W -> OK - enable local timestamp
    if(!localTimestampEnabled) {
        respPtr = executeAtCmd("AT+CLTS=1;&W", "OK");
        if(!respPtr) {
            LOG_E("%s: failed to enable local timestamp.", TAG);
            return false;
        }
    }

    // 8.  wait for unsolicited *PSNWID
    respPtr = waitForMessage("PSNWID", 5000);
    if(respPtr) {
        // PSNWID: "<mcc>", "<mnc>", "<full network name>"
        char *tempRespPtr = (char *)calloc(strlen(respPtr) + 1, 1);
        strcpy(tempRespPtr, respPtr);
        strtok(tempRespPtr, "\""); // 1st token *PSNWID: "
        int mcc = atoi(strtok(NULL, "\""));
        strtok(NULL, "\"");
        int mnc = atoi(strtok(NULL, "\""));
        strtok(NULL, "\"");
        apn = (char *)calloc(strlen(apn_list[mcc][mnc]) + 1, 1);
        strcpy(apn, apn_list[mcc][mnc]);
        LOG_I("%s: provider detected '%s', apn name '%s'.", TAG, strtok(NULL, "\""), apn);
        free(tempRespPtr);
    } else if (apn == nullptr) {
        apn = (char *)malloc(sizeof(DEFAULT_APN_NAME));
        strcpy(apn, DEFAULT_APN_NAME);
        LOG_W("%s: no provider detected, use default apn name '%s'.", TAG, apn);
    }

    // 9.  AT+CFUN=1,1 -> OK  - set full functionality  and reset MT
    if(!localTimestampEnabled) {
        respPtr = executeAtCmd("AT+CFUN=1,1", "OK", 10000);
        if(!respPtr) {
            LOG_E("%s: failed to set full functionality  and reset MT.", TAG);
            return false;
        }
    }

    // 10.  AT+CSCLK=0 -> OK - disable slow clock
    if(!executeAtCmd("AT+CSCLK=0", "OK")) {
        LOG_E("%s: failed to disable slow clock.", TAG);
        return false;
    }

    // 11. AT+CREG? -> +CREG: [0-9],[0-9] - get network registration status
    int retries = 5;
    bool registered, denied;
    for(int i = 1, registered = false, denied = false; !registered && !denied; i++) {
        delay(2000); // wait 2s for if network registration
        respPtr = executeAtCmd("AT+CREG?", "+CREG: [0-9],[0-9]");
        if(respPtr) {
            switch (respPtr[9]) // verify registration status
            {
            case '1':
            case '5':
                registered = true;
                break;
            case '3':
                denied = true;
                break;
            default:
                break;
            }
        }
        else if(i == retries) {
            waitForMessage("OK");
            LOG_E("%s: failed to get network registration status.", TAG);
            return false;
        }
    }

    // 12. wait for "OK"
    if(!waitForMessage("OK")) {
        LOG_E("%s: no OK received after AT+CREG?.", TAG);
        return false;
    }

    if(!registered) {

    // 13. AT+CPIN? -> READY(SIM PIN) - get pin status
        if(!executeAtCmd("AT+CPIN?", "READY")) {
            waitForMessage("OK");
            LOG_E("%s: failed to register - SIM PIN lock active.", TAG);
            return false;
        }

    // 14. wait for "OK"
        if(!waitForMessage("OK")) {
            LOG_E("%s: no OK received after AT+CPIN?.", TAG);
            return false;
        }
    }

    // 15. wait 5s for any unsolicite messages "SMS Ready"/"Call Ready"
    if(waitForMessage("SMS Ready", 5000)) {
        registered = true;
    } else {
        LOG_E("%s: no 'SMS Ready' received.", TAG);
        return false;
    }

    // 16. open bearer
    if(registered && !openBearer()) {
        LOG_E("%s: failed to open bearer.", TAG);
        return false;
    }

    // 17. finish
    if(registered) {
        initialized = true;
        LOG_I("%s: successfully connected to network.", TAG);
    } else {
        LOG_E("%s: failed - reason unknown.", TAG);
        return false;
    }
}

bool GsmController::getTimeDateLocation(char *timePtr, char *datePtr, double *longPtr, double *latPtr) {
    const char *TAG = "GetTimeDateLocation";
    const char *respPtr;
    char *tempRespPtr;
    char *token;

    // AT+CIPGSMLOC=1,1 -> +CIPGSMLOC:<locationcode>,<longitude>,<latitude>,<date>,<time>
    respPtr = executeAtCmd("AT+CIPGSMLOC=1,1", "+CIPGSMLOC:", 60000);
    if(respPtr) {
        // decode received string
        tempRespPtr = (char *)calloc(strlen(respPtr) + 1, 1);
        strtok(tempRespPtr, ":"); // +CIPGSMLOC
        int locCode = atoi(strtok(NULL, ",")); // <locationcode>
        if(locCode != 0) {
            LOG_E("%s: failed, AT+CIPGSMLOC return value = %d.", TAG, locCode);
            free(tempRespPtr);
            return false;
        }

        double longitude = atof(strtok(NULL, ",")); // <longitude>
        if(longPtr) {
            *longPtr = longitude;
        }

        double latitude = atof(strtok(NULL, ",")); // <latitude>
        if(latPtr) {
            *latPtr = latitude;
        }

        token = strtok(NULL, ","); // <date>
        if(datePtr) {
            strcpy(datePtr, (const char *)token);
        }

        token = strtok(NULL, ","); // <time>
        if(timePtr) {
            strcpy(timePtr, (const char *)token);
        }

        free(tempRespPtr);
        LOG_I("%s: success: %s", TAG, respPtr);
        return false;
    } 

    LOG_E("%s: failed to obtain date, time, location.", TAG);
    return false;
}




bool GsmController::verifyResponse(const char *message, const char *rgx) {
    bool res = std::regex_match(message, std::regex(rgx));
    return res;
}

const char* GsmController::waitForMessage(const char *rgx, uint32_t timeout_ms) {
    uint32_t startTime = millis();
    while(millis() < startTime + timeout_ms) {
        if(modem->messageAvailable()) {
            const char *msgPtr = modem->readMessage();
            if(msgPtr != nullptr && verifyResponse(msgPtr, rgx)) {
                LOG_D("Message received: %s", msgPtr);
                return msgPtr;
            }
        }
        delay(10);
    }

    LOG_D("Wait for message '%s' timeouted.", rgx);
    return nullptr;
}

const char* GsmController::executeAtCmd(const char *command, const char* responseRgx, uint32_t timeout_ms) {
    LOG_D("Execute AT command: %s", command);
    modem->writeMessage(command);
    const char* res = waitForMessage(responseRgx, timeout_ms);
    if(res != nullptr) {
        LOG_D("Response received: %s", res);
    } else {
        LOG_D("No response received");
    }

    return res;
}

bool GsmController::pingModem(uint8_t retries) {
    LOG_D("Pinging modem...");
        for(int i = 0; i < retries; i++) {
            if(executeAtCmd("AT", "OK", 5000)) {
                LOG_D("Ping received.");
                return true;
            }

            delay(100);
        }

        LOG_D("Ping failed.");
        return false;
    }

bool GsmController::openBearer() {
    const char *TAG = "Open bearer";
    const char *respPtr;

    // steps
    // 1. AT+SAPBR=3,1,"Contype","GPRS" -> OK - configure bearer
    // 2. AT+SAPBR=3,1,"APN","<apn>" -> OK - configure bearer
    // 3. AT+SAPBR=1,1 -> OK - open bearer
    // 4. AT+SAPBR=2,1 -> +SAPBR: - query bearer
    // 5. wait for OK

    LOG_I("%s: start GPRS connection...", TAG);

    // 1. AT+SAPBR=3,1,"Contype","GPRS" -> OK - configure bearer
    if(!executeAtCmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK")) {
        LOG_E("%s: failed to configure bearer.", TAG);
        return false;
    }

    // 2. AT+SAPBR=3,1,"APN","<apn>" -> OK - configure bearer
    char *buffer = (char *)calloc(64, 1);
    sprintf(buffer, "AT+SAPBR=3,1,\"APN\",\"%s\"", apn);
    respPtr = executeAtCmd(buffer, "OK");
    free(buffer);
    if(!respPtr) {
        LOG_E("%s: failed to configure bearer.", TAG);
        return false;
    }

    // 3. AT+SAPBR=1,1 -> OK - open bearer
    if(!executeAtCmd("AT+SAPBR=1,1", "OK")) {
        LOG_E("%s: failed to open bearer.", TAG);
        return false;
    }

    // 4. AT+SAPBR=2,1 -> +SAPBR: - query bearer
    respPtr = executeAtCmd("AT+SAPBR=2,1", "+SAPBR:1,1", 5000);
    if(!respPtr) {
        LOG_E("%s: failed to query bearer.", TAG);
        return false;
    }

    // 5. wait for OK
    if(!waitForMessage("OK")) {
        LOG_E("%s: no OK received after query bearer.", TAG);
        return false;
    }

    LOG_I("%s: connected to GPRS", TAG);
    return true;
}

bool GsmController::closeBearer() {
    const char *TAG = "Close bearer";
    const char *respPtr;

    // AT+SAPBR=0,1 -> OK - deactivate bearer profile
    if(!executeAtCmd("AT+SAPBR=0,1", "OK", 5000)) {
        LOG_E("%s: failed to deactivate bearer profile.", TAG);
        return false;
    }

    LOG_I("%s: disconnected from GPRS", TAG);
    return true;
}
