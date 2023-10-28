#include "gsm_controller.h"
#include "logger.h"
#include "apn_list.h"

GsmController::GsmController(HardwareSerial &modemUart, gpio_num_t txPin, gpio_num_t rxPin, gpio_num_t powerPin, gpio_num_t pwrKeyPin, gpio_num_t rstPin, gpio_num_t dtrPin) :
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
    
    GsmController::deinit();

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
    const char *respPtr;
    char *tempPtr = nullptr;
    char *token;
    bool tmpRes = false;
    bool localTimestampEnabled = false;
    initialized = false;

    // steps:
    // 1.  AT -> OK - verify modem state
    // 2.  power on
    // 3.  ATE0 -> OK - echo off
    // 4.  AT+CLTS=1;&W -> OK - enable local timestamp
    // 5.  AT+CSCLK=0 -> OK - disable slow clock
    // 6.  AT+CREG? -> +CREG: - get network registration status
    // 7.  wait for "OK"
    // 8.  AT+CPIN? -> READY(SIM PIN) - get pin status
    // 9.  wait for "OK"
    // 10. Get operator name and select APN
    // 11. open bearer
    // 12. finish

    // 1.  AT -> OK - modem is on/off
    LOG_V("Verify modem state...");
    tmpRes = pingModem();

    // 2.  power on
    if(!tmpRes) {
        LOG_V("No response for 'AT' command.");
        if(!powerOnModem()) {
            LOG_E("Power on failed.");
            return false;
        }
    }

    // 3.  ATE0 -> OK - echo off
    LOG_D("Switch echo off...");
    if(!executeAtCmd("ATE0", "OK")) {
        LOG_E("Echo off failed.");
        return false;
    }

    // 4.  AT+CLTS=1;&W -> OK - enable local timestamp
    LOG_D("Enable local timestamp...");
    if(!executeAtCmd("AT+CLTS=1", "OK", 5000)) {
        LOG_E("Failed to enable local timestamp.");
        return false;
    }

    // 5.  AT+CSCLK=0 -> OK - disable slow clock
    LOG_D("Disable slow clock...");
    if(!executeAtCmd("AT+CSCLK=0", "OK", 5000)) {
        LOG_E("Failed to disable slow clock.");
        return false;
    }

    // 6. AT+CREG? -> +CREG: [0-9],[0-9] - get network registration status
    LOG_D("Get network registration status...");
    int retries = 5;
    bool registered = false, denied = false;
    for(int i = 1; !registered && !denied; i++) {
        delay(2000); // wait 2s for if network registration
        respPtr = executeAtCmd("AT+CREG?", "+CREG:", 10000);
        if(respPtr) {
            switch (respPtr[9]) // verify registration status
            {
            case '1':
                registered = true;
                LOG_V("Registered in home network.");
                break;
            case '2':
                LOG_V("Searching for operator...");
                break;
            case '3':
                denied = true;
                LOG_D("Registeration denied.");
                break;
            case '5':
                registered = true;
                LOG_V("Registered in roaming.");
                break;
            default:
                LOG_D("Registeration state unknown (%c).", respPtr[9]);
                break;
            }
        }
        else if(i == retries) {
            waitForMessage("OK");
            LOG_E("Failed to get network registration status.");
            return false;
        }
    }

    // 7. wait for "OK"
    LOG_D("Wait for OK...");
    if(!waitForMessage("OK")) {
        LOG_E("No OK received after AT+CREG?.");
        return false;
    }

    if(!registered) {

    // 8. AT+CPIN? -> READY(SIM PIN) - get pin status
    LOG_D("Get pin status...");
        if(!executeAtCmd("AT+CPIN?", "READY")) {
            waitForMessage("OK");
            LOG_E("Failed to register - SIM PIN lock active.");
            return false;
        }

    // 9. wait for "OK"
        if(!waitForMessage("OK")) {
            LOG_E("No OK received after AT+CPIN?.");
            return false;
        }
    }

    // 10. Get operator name and select APN
    respPtr = executeAtCmd("AT+COPS?", "+COPS"); // +COPS: <mode>,<format>,"<oper>"
    tempPtr = (char *)calloc(strlen(respPtr) + 1, 1);
    strcpy(tempPtr, respPtr);
    strtok(tempPtr, "\""); // +COPS: <mode>,<format>,
    token = strtok(NULL, "\"");
    LOG_I("Registered with operator '%s'.", token);
    if(apn_list.find((const char *)token) == apn_list.end()) {
        LOG_E("Operator '%s' not known.", token);
        free(tempPtr);
        return false;
    }
    
    strcpy(apn, apn_list[(const char *)token]);
    LOG_I("APN '%s'.", apn);
    free(tempPtr);

    // 10. open bearer
    // if(registered && !openBearer()) {
    //     LOG_E("Failed to open bearer.");
    //     return false;
    // }

    // 11. finish
    if(registered) {
        initialized = true;
        LOG_I("Successfully connected to network.");
    } else {
        LOG_E("Failed - reason unknown.");
    }

    return registered;
}

bool GsmController::deinit() {
    if(initialized) {
        return closeBearer();
    }

    return true;
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
        return true;
    } 

    LOG_E("%s: failed to obtain date, time, location.", TAG);
    return false;
}

bool GsmController::getLocalDateTime(char *timePtr, char *datePtr) {
    const char *TAG = "GetLocalDateTime";
    const char *respPtr;
    char *tempRespPtr;
    char *token;

    // AT+CCLK? -> +CCLK: <time> - format is "yy/MM/dd,hh:mm:ss±zz"
    respPtr = executeAtCmd("AT+CCLK?", "+CCLK:");
    if(respPtr) {
        tempRespPtr = (char *)calloc(strlen(respPtr) + 1, 1);
        LOG_V("%s: decode received string.", TAG);
        token = strtok(tempRespPtr, "\""); // first token (+CCLK: ")
        token  = strtok(NULL, ","); // date (yy/MM/dd)
        LOG_V("Date: %s", token);
        if(datePtr) { // new format yyyy/MM/dd
            datePtr[0] = '2';
            datePtr[1] = '0';
            datePtr[2] = token[0];
            datePtr[3] = token[1];
            datePtr[4] = '/';
            datePtr[5] = token[3];
            datePtr[6] = token[4];
            datePtr[7] = '/';
            datePtr[8] = token[6];
            datePtr[9] = token[7];
            LOG_V("Formatted date: %s", datePtr);
        }

        token = strtok(NULL, "+-"); // time (hh:mm:ss)
        LOG_V("Time: %s", token);
        if(timePtr) {
            strcpy(timePtr, token);
        }

        free(tempRespPtr);
        LOG_I("%s: success: %s", TAG, respPtr);
        return true;
    }

    LOG_E("%s: failed to obtain local time.", TAG);
    return false;
}

bool GsmController::sendHttpGetReq(const char *url, char *payload) {
    const char *TAG = "SendHttpGetReq";
    const char *respPtr;
    char *tempPtr = nullptr;
    char *token;
    int payloadLen = 0;
    bool res;

    // steps
    // 1.  AT+HTTPINIT -> OK                - Initialize HTTP Service
    // 2.  AT+HTTPPARA="CID",1 -> OK        - Set HTTP Parameters Value
    // 3.  AT+HTTPPARA="URL","<url>" -> OK  - Set HTTP Parameters Value
    // 4.  AT+HTTPACTION=0 -> OK            - HTTP Method Action
    // 5.  wait for +HTTPACTION: <Method>,<StatusCode>,<DataLen>
    // 6.  decode HTTPACTION status
    // 7.  AT+HTTPREAD -> +HTTPREAD:        - Read the HTTP Server Response
    // 8.  read http response
    // 9.  wait for OK
    // 10. AT+HTTPTERM -> OK                - Terminate HTTP Service
    // 11. finish

    LOG_I("%s: requesting url '%s'", TAG, url);

    // 1.  AT+HTTPINIT -> OK                - Initialize HTTP Service
    if(!executeAtCmd("AT+HTTPINIT", "OK")) {
        LOG_E("%s: failed to initialize HTTP service.", TAG);
        return false;
    }

    // 2.  AT+HTTPPARA="CID",1 -> OK        - Set HTTP Parameters Value
    if(!executeAtCmd("AT+HTTPPARA=\"CID\",1", "OK")) {
        LOG_E("%s: failed to set HTTP CID parameter.", TAG);
        return false;
    }

    // 3.  AT+HTTPPARA="URL","<url>" -> OK  - Set HTTP Parameters Value
    const char *cmdFmt = "AT+HTTPPARA=\"URL\",\"%s\"";
    tempPtr = (char *)calloc(strlen(cmdFmt) + strlen(url) + 1, 1);
    sprintf(tempPtr, cmdFmt, url);
    res = executeAtCmd((const char *)tempPtr, "OK") != nullptr;
    if(!res) {
        LOG_E("%s: failed to set HTTP URL parameter.", TAG);
    }
    
    // 4.  AT+HTTPACTION=0 -> OK            - HTTP Method Action (GET)
    if(res) {
        res = executeAtCmd("AT+HTTPACTION=0", "OK", 5000) != nullptr;
        if(!res) {
            LOG_E("%s: failed to set HTTP Method Action (GET).", TAG);
        }
    }

    // 5.  wait for +HTTPACTION: <Method>,<StatusCode>,<DataLen>
    if(res) {
        respPtr = waitForMessage("+HTTPACTION", 5000);
        res = respPtr != nullptr;
        if(!res) {
            LOG_E("%s: no response for HTTP Action GET.", TAG);
        }
    }
    
    // 6.  decode HTTPACTION status
    if(res) {
        tempPtr = (char *)realloc(tempPtr, strlen(respPtr) + 1);
        strcpy(tempPtr, respPtr);
        token = strtok(tempPtr, ","); // +HTTPACTION: <Method>
        token = strtok(NULL, ","); // <StatusCode>
        if(atoi(token) != 200)
        {
            res = false;
            LOG_E("%s: failed with HTTP code %s.", TAG, token);
        } else {
            token = strtok(NULL, ","); // <DataLen>
            payloadLen = atoi(token);
        }
    }

    // 7.  AT+HTTPREAD -> +HTTPREAD:        - Read the HTTP Server Response
    if(res && payloadLen > 0) {
        res = executeAtCmd("AT+HTTPREAD", "+HTTPREAD:", 5000) != nullptr;
        if(!res) {
            LOG_E("%s: failed to obtain HTTP response.", TAG);
        }
    }

    // 8.  read http response
    if(res && payloadLen > 0) {
        respPtr = waitForMessage(nullptr, 5000);
        res = respPtr != nullptr;
        if(!res) {
            LOG_E("%s: HTTP response cannot be read.", TAG);
        } else {
            strcpy(payload, respPtr);
        }
    }

    // 9.  wait for OK
    if(res && payloadLen > 0) {
        if(!waitForMessage("OK")) {
            LOG_E("%s: no OK after read of HTTP response.", TAG);
        }
    }

    // 10. AT+HTTPTERM -> OK                - Terminate HTTP Service
    if(!executeAtCmd("AT+HTTPTERM", "OK")) {
        LOG_E("%s: failed to terminate HTTP Service.", TAG);
    }

    // 11. finish
    if(tempPtr) {
        free(tempPtr);
    }

    if(res) {
        LOG_I("%s: success. Payload:\n%s", TAG, payload);
    }

    return res;
}

bool GsmController::verifyResponse(const char *message, const char *substr) {
    LOG_D("Verification '%s' vs '%s'", message, substr);
    bool res = strstr(message, substr) != nullptr;
    LOG_D("Verification %s", res ? "passed" : "not passed");
    return res;
}

const char* GsmController::waitForMessage(const char *rgx, uint32_t timeout_ms) {
    uint32_t startTime = millis();
    while(millis() < startTime + timeout_ms) {
        if(modem->messageAvailable()) {
            const char *msgPtr = modem->readMessage();
            if((msgPtr != nullptr) && (!rgx || verifyResponse(msgPtr, rgx))) {
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
    executeAtCmd("AT");
    for(int i = 0; i < retries; i++) {
        delay(100);
        if(executeAtCmd("AT", "OK")) {
            LOG_D("Ping received.");
            return true;
        }
    }

    LOG_D("Ping failed.");
    return false;
}

bool GsmController::openBearer() {
    const char *respPtr;

    // steps
    // 1. AT+SAPBR=3,1,"Contype","GPRS" -> OK - configure bearer
    // 2. AT+SAPBR=3,1,"APN","<apn>" -> OK - configure bearer
    // 3. AT+SAPBR=1,1 -> OK - open bearer
    // 4. AT+SAPBR=2,1 -> +SAPBR: - query bearer
    // 5. wait for OK

    LOG_I("Start GPRS connection...");

    // 1. AT+SAPBR=3,1,"Contype","GPRS" -> OK - configure bearer
    LOG_D("Set connection type GPRS...");
    if(!executeAtCmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK")) {
        LOG_E("Failed to configure bearer.");
        return false;
    }

    // 2. AT+SAPBR=3,1,"APN","<apn>" -> OK - configure bearer

    char *buffer = (char *)calloc(64, 1);
    sprintf(buffer, "AT+SAPBR=3,1,\"APN\",\"%s\"", apn);
    respPtr = executeAtCmd(buffer, "OK");
    free(buffer);
    if(!respPtr) {
        LOG_E("Failed to configure bearer.");
        return false;
    }

    // 3. AT+SAPBR=1,1 -> OK - open bearer
    if(!executeAtCmd("AT+SAPBR=1,1", "OK")) {
        LOG_E("Failed to open bearer.");
        return false;
    }

    // 4. AT+SAPBR=2,1 -> +SAPBR: - query bearer
    respPtr = executeAtCmd("AT+SAPBR=2,1", "+SAPBR:1,1", 5000);
    if(!respPtr) {
        LOG_E("Failed to query bearer.");
        return false;
    }

    // 5. wait for OK
    if(!waitForMessage("OK")) {
        LOG_E("No OK received after query bearer.");
        return false;
    }

    LOG_I("Connected to GPRS.");
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
