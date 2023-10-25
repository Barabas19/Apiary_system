#include "sim800.h"

SIM800::SIM800(HardwareSerial &uart, gpio_num_t txPin, gpio_num_t rxPin, gpio_num_t powerPin, gpio_num_t pwrKeyPin, gpio_num_t rstPin, gpio_num_t dtrPin) :
    uart{uart},
    txPin{txPin},
    rxPin{rxPin},
    powerPin{powerPin},
    pwrKeyPin{pwrKeyPin},
    rstPin{rstPin},
    dtrPin{dtrPin}
{
    if(powerPin != GPIO_NUM_NC) {
        gpio_set_direction(powerPin, GPIO_MODE_OUTPUT);
        gpio_set_level(powerPin, LOW);
    }

    if(pwrKeyPin != GPIO_NUM_NC) {
        gpio_set_direction(pwrKeyPin, GPIO_MODE_OUTPUT);
        gpio_set_level(pwrKeyPin, HIGH);
    }

    if(rstPin != GPIO_NUM_NC) {
        gpio_set_direction(rstPin, GPIO_MODE_OUTPUT);
        gpio_set_level(rstPin, HIGH);
    }

    if(dtrPin != GPIO_NUM_NC) {
        gpio_set_direction(dtrPin, GPIO_MODE_OUTPUT);
        gpio_set_level(dtrPin, LOW);
    }
    
    uart.setRxBufferSize(RX_MAX_BUFFER_SIZE);
    uart.begin(115200, SERIAL_8N1, txPin, rxPin);

    // allocate memory for read buffer
    readBufferSize = RX_DEFAULT_BUFFER_SIZE + 1;
    readBuffer = (char *)malloc(readBufferSize);
}

SIM800::~SIM800() {
    uart.end();
    free(readBuffer);
}

void SIM800::powerOn() {
    if(powerPin != GPIO_NUM_NC) {
        gpio_set_level(powerPin, HIGH);
    }

    if(pwrKeyPin != GPIO_NUM_NC) {
        gpio_set_level(pwrKeyPin, LOW);
        delay(1000);
        gpio_set_level(pwrKeyPin, HIGH);
        delay(2000);
    }
}

void SIM800::powerOff() {
    if(pwrKeyPin != GPIO_NUM_NC) {
        gpio_set_level(pwrKeyPin, LOW);
        delay(1000);
        gpio_set_level(pwrKeyPin, HIGH);
        delay(2000);
    }

    if(powerPin != GPIO_NUM_NC) {
        gpio_set_level(powerPin, LOW);
    }

    delay(2000);

    uart.flush();
    while(SIM800::messageAvailable()) {
        SIM800::readMessage();
    }
}

bool SIM800::messageAvailable() {
    return uart.available();
}

const char* SIM800::readMessage() {
    int received = 0;
    char c;
    if(!messageAvailable()) {
        return nullptr;
    }

    readBuffer = (char *)memset(readBuffer, 0, readBufferSize);

    while(true){
        // extend buffer size, if necessary
        if(received >= readBufferSize - 2) {
            int newSize = min(readBufferSize + RX_DEFAULT_BUFFER_SIZE, RX_MAX_BUFFER_SIZE + 1);
            readBuffer = (char *)realloc(readBuffer, newSize);
            memset(&(readBuffer[readBufferSize]), 0, newSize - readBufferSize);
            readBufferSize = newSize;
        }

        // read character
        if(uart.readBytes(&c, 1) == 0) {
            break;
        }

        if(c == '\r') {
            continue;
        }

        if(c == '\n') {
            break;
        }

        readBuffer[received++] = c;
    }
    
    return received > 0 ? readBuffer : nullptr;
}

void SIM800::writeMessage(const char *message) {
    uart.print(message);
    uart.write('\r');
}