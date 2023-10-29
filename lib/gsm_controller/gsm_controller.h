#ifndef GSMCONTROLLER_H
#define GSMCONTROLLER_H

#include <Arduino.h>
#include <time.h>

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

    /** Disconnect from network
     * @returns - true, if successfully deinitialized
    */
    bool deinit();

    /** Get date, time and location from the network
     * @dt - time structure
     * @longPtr - pointer, where the longitude should be copied to
     * @latPtr - pointer, where the latitude should be copied to
     * @returns - true, if successfully received
    */
    bool getTimeDateLocation(struct tm &dt, double *longPtr = nullptr, double *latPtr = nullptr);
    
    /** Get local date and time
     * @dt - time structure
     * @returns - true, if successfully received
    */
    bool getLocalDateTime(struct tm &dt);
    
    /** Send HTTP GET request
     * @url - full url to be sent
     * @payload - payload returned
     * @maxPayloadSize - the maximum payload size to be read
     * @returns - true, if successfully sent
    */
    bool sendHttpGetReq(const char *url, char *payload, const uint16_t maxPayloadSize);
    
private:
    SIM800 *modem;
    char apn[32];
    bool initialized;

    /** Check the message, if contains a substring
     * @message - the message to be verified
     * @substr - substring
     * @returns - true, if substring found in the message
    */
    bool verifyResponse(const char *message, const char *substr);

    /** Wait for message from modem
     * @msgSubstr - substring of the message, that should be received. Use nullptr to read any message
     * @returns - pointer to the desired message or nullptr, if nothing received
    */
    const char* waitForMessage(const char *msgSubstr = nullptr, uint32_t timeout_ms = 1000);

    /** Execute AT command
     * @command - command to be executed
     * @respSubstr - substring of the response, that should be received, or nullptr to do not check the response
     * @timeout_ms - execution timeout in ms
     * @returns - pointer to the response or nullptr, if nothing received
    */
    const char* executeAtCmd(const char *command, const char* respSubstr = nullptr, uint32_t timeout_ms = 1000);

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