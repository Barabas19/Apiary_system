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
    
    if(initialized) {
        deinit();
    }

    modem->powerOff();
    bool res = !pingModem();
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
    // 8.  AT+COPS? -> +COPS: <mode>,<format>,"<oper>" - Get operator name and select APN
    // 9.  wait for "OK"
    // 10. open bearer
    // 11. finish

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

    // 8. AT+COPS? -> +COPS: <mode>,<format>,"<oper>" - Get operator name and select APN
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

    // 9. wait for "OK"
    LOG_D("Wait for OK...");
    if(!waitForMessage("OK")) {
        LOG_E("No OK received after AT+CREG?.");
        return false;
    }


    // 10. open bearer
    if(registered && !openBearer()) {
        LOG_E("Failed to open bearer.");
        return false;
    }

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
        initialized = false;
        return closeBearer();
    }

    return true;
}

bool GsmController::getTimeDateLocation(struct tm &dt, double *longPtr, double *latPtr) {
    const char *respPtr;
    char *tempRespPtr;
    char *token;

    // AT+CIPGSMLOC=1,1 -> +CIPGSMLOC:<locationcode>,<longitude>,<latitude>,<date>,<time>
    respPtr = executeAtCmd("AT+CIPGSMLOC=1,1", "+CIPGSMLOC:", 60000);
    if(respPtr) {
        // decode received string
        tempRespPtr = (char *)calloc(strlen(respPtr) + 1, 1);
        strcpy(tempRespPtr, respPtr);
        strtok(tempRespPtr, " "); // +CIPGSMLOC:
        int locCode = atoi(strtok(NULL, ",")); // <locationcode>
        LOG_D("Location code: %d", locCode);
        if(locCode != 0) {
            LOG_E("AT+CIPGSMLOC return value = %d.", locCode);
            free(tempRespPtr);
            return false;
        }

        double longitude = atof(strtok(NULL, ",")); // <longitude>
        LOG_D("Longitude: %.5f", longitude);
        if(longPtr) {
            *longPtr = longitude;
        }

        double latitude = atof(strtok(NULL, ",")); // <latitude>
        LOG_D("Latitude: %.5f", latitude);
        if(latPtr) {
            *latPtr = latitude;
        }

        // <date> format YYYY/MM/DD
        dt.tm_year = atoi(strtok(NULL, "/"));
        dt.tm_mon = atoi(strtok(NULL, "/"));
        dt.tm_mday = atoi(strtok(NULL, ","));
        // <time> format hh/mm/ss
        dt.tm_hour = atoi(strtok(NULL, ":"));
        dt.tm_min = atoi(strtok(NULL, ":"));
        dt.tm_sec = atoi(strtok(NULL, ":"));

        free(tempRespPtr);
        LOG_I("Date, time, location: %s", respPtr);
        return true;
    } 

    LOG_E("Failed to obtain date, time, location.");
    return false;
}

bool GsmController::getLocalDateTime(struct tm &dt) {
    const char *respPtr;
    char *tempRespPtr;
    char *token;

    // AT+CCLK? -> +CCLK: <time> - format is "yy/MM/dd,hh:mm:ss±zz"
    respPtr = executeAtCmd("AT+CCLK?", "+CCLK:");
    if(respPtr) {
        tempRespPtr = (char *)calloc(strlen(respPtr) + 1, 1);
        strcpy(tempRespPtr, respPtr);
        LOG_V("Decode received string.");
        token = strtok(tempRespPtr, "\""); // first token (+CCLK: ")
        dt.tm_year = atoi(strtok(NULL, "/")) + 2000;
        dt.tm_mon = atoi(strtok(NULL, "/"));
        dt.tm_mday = atoi(strtok(NULL, ","));
        dt.tm_hour = atoi(strtok(NULL, ":"));
        dt.tm_min = atoi(strtok(NULL, ":"));
        dt.tm_sec = atoi(strtok(NULL, "+-"));
        free(tempRespPtr);
        LOG_I("Local date time: %s", respPtr);
        return true;
    }

    LOG_E("Failed to obtain local time.");
    return false;
}

bool GsmController::sendHttpGetReq(const char *url, char *payload, const uint16_t maxPayloadSize) {
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

    LOG_I("Requesting url '%s'...", url);

    // 1.  AT+HTTPINIT -> OK                - Initialize HTTP Service
    if(!executeAtCmd("AT+HTTPINIT", "OK")) {
        LOG_E("Failed to initialize HTTP service.");
        return false;
    }

    // 2.  AT+HTTPPARA="CID",1 -> OK        - Set HTTP Parameters Value
    if(!executeAtCmd("AT+HTTPPARA=\"CID\",1", "OK")) {
        LOG_E("Failed to set HTTP CID parameter.");
        return false;
    }

    // 3.  AT+HTTPPARA="URL","<url>" -> OK  - Set HTTP Parameters Value
    const char *cmdFmt = "AT+HTTPPARA=\"URL\",\"%s\"";
    tempPtr = (char *)calloc(strlen(cmdFmt) + strlen(url) + 1, 1);
    sprintf(tempPtr, cmdFmt, url);
    res = executeAtCmd((const char *)tempPtr, "OK") != nullptr;
    if(!res) {
        LOG_E("Failed to set HTTP URL parameter.");
    }
    
    // 4.  AT+HTTPACTION=0 -> OK            - HTTP Method Action (GET)
    if(res) {
        res = executeAtCmd("AT+HTTPACTION=0", "OK", 5000) != nullptr;
        if(!res) {
            LOG_E("Failed to set HTTP Method Action (GET).");
        }
    }

    // 5.  wait for +HTTPACTION: <Method>,<StatusCode>,<DataLen>
    if(res) {
        respPtr = waitForMessage("+HTTPACTION", 10000);
        res = respPtr != nullptr;
        if(!res) {
            LOG_E("No response for HTTP Action GET.");
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
            LOG_E("Failed with HTTP code %s.", token);
        } else {
            token = strtok(NULL, ","); // <DataLen>
            payloadLen = atoi(token);
        }
    }

    int payloadSize = min(maxPayloadSize - 1, payloadLen);

    // 7.  AT+HTTPREAD -> +HTTPREAD:        - Read the HTTP Server Response
    if(res && payloadLen > 0) {
        tempPtr = (char *)realloc(tempPtr, 32);
        sprintf(tempPtr, "AT+HTTPREAD=0,%u", payloadSize);
        res = executeAtCmd(tempPtr, "+HTTPREAD:", 10000) != nullptr;
        if(!res) {
            LOG_E("Failed to obtain HTTP response.");
        }
    }

    // 8.  read http response
    if(res && payloadLen > 0) {
        respPtr = waitForMessage(nullptr, 5000);
        res = respPtr != nullptr;
        if(!res) {
            LOG_E("HTTP response cannot be read.");
        } else {
            strcpy(payload, respPtr);
        }
    }

    // 9.  wait for OK
    if(res && payloadLen > 0) {
        if(!waitForMessage("OK")) {
            LOG_E("No OK after read of HTTP response.");
        }
    }

    // 10. AT+HTTPTERM -> OK                - Terminate HTTP Service
    if(!executeAtCmd("AT+HTTPTERM", "OK")) {
        LOG_E("Failed to terminate HTTP Service.");
    }

    // 11. finish
    if(tempPtr) {
        free(tempPtr);
    }

    if(res) {
        LOG_I("Payload received:\n%s", payload);
    }

    return res;
}

bool GsmController::sendHttpPostReq(const char *url, const char *data) {
    const char *respPtr;
    char *tempPtr = nullptr;
    char *token;
    int payloadLen = 0;
    bool res;

    // steps
    // 1.  AT+HTTPINIT -> OK                - Initialize HTTP Service
    // 2.  AT+HTTPPARA="CID",1 -> OK        - Set HTTP Parameters Value
    // 3.  AT+HTTPPARA="URL","<url>" -> OK  - Set HTTP Parameters Value
    // 4.  AT+HTTPPARA="CONTENT","application/x-www-form-urlencoded" -> OK  - Set HTTP Parameters Value
    // 5.  AT+HTTPDATA=<size>,<time> -> DOWNLOAD - Input HTTP Data
    // 6.  write POST data
    // 7.  wait for OK
    // 8.  AT+HTTPACTION=1 -> OK            - HTTP Method Action
    // 9.  wait for +HTTPACTION: <Method>,<StatusCode>,<DataLen>
    // 10. decode HTTPACTION status
    // 11. AT+HTTPTERM -> OK                - Terminate HTTP Service
    // 12. finish
}

bool GsmController::verifyResponse(const char *message, const char *substr) {
    LOG_D("Verification '%s' vs '%s'", message, substr);
    bool res = strstr(message, substr) != nullptr;
    LOG_D("Verification %s", res ? "passed" : "not passed");
    return res;
}

const char* GsmController::waitForMessage(const char *msgSubstr, uint32_t timeout_ms) {
    uint32_t startTime = millis();
    while(millis() < startTime + timeout_ms) {
        if(modem->messageAvailable()) {
            const char *msgPtr = modem->readMessage();
            if(msgPtr == nullptr) {
                continue;
            }

            if(msgSubstr == nullptr || verifyResponse(msgPtr, msgSubstr)) {
                LOG_D("Message received: %s", msgPtr);
                return msgPtr;
            }
        }
        delay(10);
    }

    LOG_D("Wait for message '%s' timeouted.", msgSubstr ? msgSubstr : "");
    return nullptr;
}

const char* GsmController::executeAtCmd(const char *command, const char* respSubstr, uint32_t timeout_ms) {
    LOG_D("'%s' -> '%s'", command, (respSubstr ? respSubstr : ""));
    modem->writeMessage(command);
    const char* res = waitForMessage(respSubstr, timeout_ms);
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
        if(executeAtCmd("AT", "OK", 200)) {
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
    if(!executeAtCmd("AT+SAPBR=1,1", "OK", 10000)) {
        LOG_E("Failed to open bearer.");
        return false;
    }

    // 4. AT+SAPBR=2,1 -> +SAPBR: - query bearer
    respPtr = executeAtCmd("AT+SAPBR=2,1", "+SAPBR: 1,1", 5000);
    if(!respPtr) {
        LOG_E("Failed to query bearer.");
        return false;
    }

    // 5. wait for OK
    if(!waitForMessage("OK")) {
        LOG_E("No OK received after query bearer.");
        return false;
    }

    LOG_I("Successfully connected to GPRS.");
    return true;
}

bool GsmController::closeBearer() {
    const char *respPtr;

    // AT+SAPBR=0,1 -> OK - deactivate bearer profile
    if(!executeAtCmd("AT+SAPBR=0,1", "OK", 5000)) {
        LOG_E("Failed to deactivate bearer profile.");
        return false;
    }

    LOG_I("Successfully disconnected from GPRS.");
    return true;
}
