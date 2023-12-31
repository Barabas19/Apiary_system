#ifndef SIM800_H_
#define SIM800_H_

#include <Arduino.h>


#define RX_MAX_BUFFER_SIZE      1460

class SIM800
{
public:
    /** Default constructor
     * @uart - UART for communication with SIM800L
     * @txPin - pin number, which the TXD pin of SIM800L is connected to
     * @rxPin - pin number, which the RXD pin of SIM800L is connected to
     * @powerPin - pin number for control SIM800 power supply
     * @pwrKeyPin - pin number, which the PWRKEY pin of SIM800L is connected to
     * @rstPin - pin number, which the RST pin of SIM800L is connected to
     * @dtrPin - pin number, which the DTR pin of SIM800L is connected to
     */
    SIM800(HardwareSerial &uart, gpio_num_t txPin, gpio_num_t rxPin, gpio_num_t powerPin, gpio_num_t pwrKeyPin, gpio_num_t rstPin, gpio_num_t dtrPin);

    ~SIM800();

    /** Power on */
    void powerOn();

    /** Power off */
    void powerOff();

    /** Check, if any character is available on SIM800Ls UART
     * @returns - true, if a character is available
    */
    bool messageAvailable();

    /** Read message from SIM800Ls UART
     * Any line of the message is read separately: <CR><LF>message<CR><LF>
     * @returns - pointer to the message. In case of no message, nullptr is returned
     */
    const char* readMessage();

    /** Write message to SIM800Ls UART
     * @message - message to be sent
     */
    void writeMessage(const char *message);

private:
    HardwareSerial &uart;
    gpio_num_t txPin;
    gpio_num_t rxPin;
    gpio_num_t powerPin;
    gpio_num_t pwrKeyPin;
    gpio_num_t rstPin;
    gpio_num_t dtrPin;
    char readBuffer[RX_MAX_BUFFER_SIZE];
};

#endif // SIM800_H_