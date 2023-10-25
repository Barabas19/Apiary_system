#ifndef GSMCONTROLLER_H
#define GSMCONTROLLER_H

#include <Arduino.h>

#include "sim800.h"

class GsmController {
public:
    /** Default constructor
     * @modemUart - UART, which the modem is connected to
     */
    GsmController(HardwareSerial &modemUart, gpio_num_t txPin, gpio_num_t rxPin, gpio_num_t powerPin, gpio_num_t pwrKeyPin, gpio_num_t rstPin, gpio_num_t dtrPin);
    
    /** Power on modem
     * @returns - true, if successfully powered on
    */
    bool powerOnModem();

    /** Power off modem 
     * @returns - true, if successfully powered off
    */
    bool powerOffModem();

    /** Initialize, connect modem to network
     * @returns - true, if successfully initialized
    */
    bool init();

    bool deinit();

    /** Get date, time and location from the network
     * @timePtr - pointer, where the time should be copied to. Format hh/mm/ss
     * @datePtr - pointer, where the date should be copied to. Format YYYY/MM/DD
     * @longPtr - pointer, where the longitude should be copied to
     * @latPtr - pointer, where the latitude should be copied to
     * @returns - true, if successfully received
    */
    bool getTimeDateLocation(char *timePtr, char *datePtr = nullptr, double *longPtr = nullptr, double *latPtr = nullptr);
    
    bool getLocalDateTime(char *timePtr, char *datePtr = nullptr);
    int getBatteryLevel();
    bool sendHttpGetReq(const char *url, char *payload);
    
private:
    SIM800 *modem;
    char *apn;
    bool initialized;

    /** Check the message due to regex
     * @message - the message to be verified
     * @rgx - regex
     * @returns - true, if regex found in the message
    */
    bool verifyResponse(const char *message, const char *rgx);

    /** Wait for message from modem
     * @rgx - regex of the message, that should be received
     * @returns - pointer to the desired message or nullptr, if nothing received
    */
    const char* waitForMessage(const char *rgx, uint32_t timeout_ms = 1000);

    /** Execute AT command
     * @command - command to be executed
     * @responseRgx - regex of the response, that should be received, or nullptr to do not check the response
     * @timeout_ms - execution timeout in ms
     * @returns - pointer to the response or nullptr, if nothing received
    */
    const char* executeAtCmd(const char *command, const char* responseRgx = nullptr, uint32_t timeout_ms = 1000);

    /** Send several AT commands to verify modem state
     * @retries - number of retries
     * @returns - true, if modem answers
    */
    bool pingModem(uint8_t retries = 3);

    /** Activate bearer profile - open GPRS connection
     * @returns - true, if successfully activated
    */
    bool openBearer();
    
    /** Deactivate bearer profile - close GPRS connection
     * @returns - true, if successfully deactivated
    */
    bool closeBearer();

};

#endif // GSMCONTROLLER_H