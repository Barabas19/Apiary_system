#ifndef SIM800_H_
#define SIM800_H_

#define RX_BUFFER_SIZE 1460

#include <ArduinoHttpClient.h>
#include <StreamDebugger.h>

class SIM800
{
    public:

    // TODO: create check of incoming / missed call - may be used AT+CLCC

    enum ATCmdStatus
    {
        RESP_WAITING,
        RESP_RECEIVED,
        RESP_RECEIVED1,
        RESP_RECEIVED2,
        RESP_RECEIVED3,
        RESP_TIMEOUT,
        RESP_AVAILABLE,
        CMD_SENT,
        ERROR
    };

    enum SeqStatus
    {
        RUNNING,
        FINISHED_SUCSESS,
        FINISHED_FAILED
    };

    /** Default constructor
     * @_atStream - UART, which SIM800L is connected to
     * @_resetPin - pin number, which the RST pin of SIM800L is connected to
     * @_dtrPin - pin number, which the DTR pin of SIM800L is connected to
     * @_apn - name of GPRS APN
     */
    SIM800(HardwareSerial& _atStream, uint8_t _resetPin, uint8_t _dtrPin, const char* _apn);

    /** Sends AT command to SIM800L
     * @_cmd - command to be send
     * @_resp_timeout_ms - timeout for waiting for response
     * @_rcvBuff - if not null, the received string will be copied into
     * @_resp1, _resp2, _resp3 - possible responses. Are evaluated as substring of the response.
     * @returns - status of the sent command
     */
    ATCmdStatus executeATCommand(const char* _cmd, uint32_t _resp_timeout_ms = 0, char* _rcvBuff = nullptr,
        const char* _resp1 = nullptr, const char* _resp2 = nullptr, const char* _resp3 = nullptr);

    /** Waits for a message for SIM800L
     * @_resp_timeout_ms - timeout for waiting for response
     * @_rcvBuff - if not null, the received string will be copied into
     * @_resp1, _resp2, _resp3 - possible responses. Are evaluated as substring of the response.
     * @returns - status of the sent command
     */
    ATCmdStatus waitForMessage(uint32_t _resp_timeout_ms = 1000, char* _rcvBuff = nullptr,
        const char* _resp1 = nullptr, const char* _resp2 = nullptr, const char* _resp3 = nullptr);
    
    /** Sends a message to SIM800L
     * @_message - message to be sent
     */
    void writeMessage(const char* _message);

    /** Reads any message from SIM800L.
     * Any line of the message is read separately: <CR><LF>message<CR><LF>
     * @_rcvBuff - if not null, the received string will be copied into
     * @_cr_ln_repeats_to_break - the functions returns after <CR><LF> repeats (once, twice, ...)
     * @returns - true, if any message received
     */
    // bool ReadMessage(char* _rcvBuff = nullptr, uint _cr_ln_repeats_to_break = 1);
    uint16_t readMessage(char* _rcvBuff = nullptr, const char* _break_string = "\r\n", bool _remove_break_string = true);

    /** Enters SIM800L into sleep mode
     * @returns - status of the sequence
     */
    SeqStatus enterSleepMode();

    /** Initialize SIM800L
     * @returns - status of the sequence
     */
    SeqStatus init();

    SeqStatus power_on_and_wait_for_ready();
    SeqStatus power_off_with_delay(uint64_t _delay_ms);

    /** Gets date, time and location of GSM cell
     * @_timeBuff - buffer for time string in format 'hh:mm:ss'
     * @_dateBuff - buffer for date string in format 'dd.MM.yyyy'
     * @_longBuff - buffer for longitude of the location '121.354848'
     * @_latBuff - buffer for latitude of the location '121.354848'
     * @returns - status of the sequence
     */
    SeqStatus getDateTimeLoc(char* _timeBuff, char* _dateBuff = nullptr, char* _longBuff = nullptr, char* _latBuff = nullptr);

    /** Gets local date and time
     * @_timeBuff - buffer for time string in format 'hh:mm:ss'
     * @_dateBuff - buffer for date string in format 'dd.MM.yyyy'
     * @returns - status of the sequence
     */
    SeqStatus getLocalTimeStamp(char* _timeBuff, char* _dateBuff = nullptr);

    /** Gets battery charge level in percent
     * @returns - percentage of charge level
     */
    SeqStatus getBattLevel(uint8_t *_level);

    /** Make a call to a number
     * @_number - number to call to
     * @returns - status of the sequence
     */
    SeqStatus makeCall(const char* _number);

    /** Make a call to myNumber
     * @returns - status of the sequence
     */
    SeqStatus makeCallToMyNumber();

    /** Verify, that incoming call is made from my number
     * @_is_my_number - reasult of the verification
     * @returns - status of the sequence
     */
    SeqStatus verifyCallFromMyNumber(bool *_is_my_number);

    /** Send HTTP GET request
     * @_url - the complete url
     * @_response - if no null, the HTTP response will be copied here
     * @returns - status of the sequence
     */
    SeqStatus sendHttpGetRequest(const char* _url, char* _response = nullptr);

    /** Connect to a TCP server, send a HTTP GET request, receive a response, download a file.
     * @_server - TCP server IP address or domain name
     * @_port - port number
     * @_send_message - HTTP GET request to be sent
     * @_received_message - buffer for a received message
     * @_file_path - path for a downloaded file
     * @returns - status of the sequence
     */
    SeqStatus sendHttpGetRequest(const char* _server, const uint16_t _port, const char* _send_message, char* _received_message = nullptr, const char* _file_path = nullptr);

    /** Download file from FTP
     * @_server - FTP server address (IP or dinamic name)
     * @_user - FTP user name
     * @_pass - FTP user password
     * @_ftp_dir - remote directory
     * @_file_name - file name
     * @_dest_dir - local destination directory
     * @_file_info - remote file info, if any exists
     * @_file_downloaded - file download is finished (is set when the file has been closed)
     * @_original_file_info - the file will be downloaded, if its info is different from the original one (nullptr = download without comparisson)
     */
    SeqStatus downloadFtpFile(const char* _server, const char* _user, const char* _pass, const char* _ftp_dir, 
        const char* _file_name, const char* _dest_dir, char* _file_info, bool& _file_downloaded, const char* _original_file_info = nullptr);
    
    /** Reset the module using PWRKEY pin or RESET pin
     * @returns - sequence status
     */
    SeqStatus reset();

    bool power_off();
    bool power_on();

    // ########################## PRIVATE ##################################
    private:
    HardwareSerial& atSerial;
    const char* apn;
    char httpData[2048];
    char rcvBuff[1024];

    // Settings
    const char* settingNames[2];

    // private methods
    bool wait(uint32_t _time);
    void readSettings();
    SeqStatus activateBearerProfile();
    SeqStatus deactivateBearerProfile();
    ATCmdStatus waitForAtSerialAvailable(uint32_t _resp_timeout_ms = 1000);
};

#endif // SIM800_H_