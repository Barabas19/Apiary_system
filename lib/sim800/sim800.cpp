#include <sim800.h>
#include <logger.h>

SIM800::SIM800(HardwareSerial &atSerial, gpio_num_t txPin, gpio_num_t rxPin, gpio_num_t powerPin, gpio_num_t pwrKeyPin, gpio_num_t rstPin, gpio_num_t dtrPin) :
    atSerial{atSerial},
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
    
    atSerial.setRxBufferSize(RX_MAX_BUFFER_SIZE);
    atSerial.begin(115200, SERIAL_8N1, txPin, rxPin);

    // allocate memory for read buffer
    readBufferSize = RX_DEFAULT_BUFFER_SIZE + 1;
    readBuffer = (char *)malloc(readBufferSize);
}

SIM800::~SIM800() {
    atSerial.end();
    free(readBuffer);
}

void SIM800::powerOn() {
    if(powerPin != GPIO_NUM_NC) {
        gpio_set_level(powerPin, HIGH);
    }

    if(pwrKeyPin != GPIO_NUM_NC) {
        gpio_set_direction(pwrKeyPin, GPIO_MODE_OUTPUT);
        gpio_set_level(pwrKeyPin, LOW);
        delay(1000);
        gpio_set_level(pwrKeyPin, HIGH);
        delay(2000);
    }

}

void SIM800::powerOff() {
    if(pwrKeyPin != GPIO_NUM_NC) {
        gpio_set_direction(pwrKeyPin, GPIO_MODE_OUTPUT);
        gpio_set_level(pwrKeyPin, LOW);
        delay(1000);
        gpio_set_level(pwrKeyPin, HIGH);
        delay(2000);
    }

    if(powerPin != GPIO_NUM_NC) {
        gpio_set_level(powerPin, LOW);
    }
}

bool SIM800::messageAvailable() {
    return atSerial.available();
}

char* SIM800::readMessage() {
    int received = 0;
    char c;
    if(!messageAvailable()) {
        return nullptr;
    }

    while(true){
        // extend buffer size, if necessary
        if(received >= readBufferSize - 2) {
            readBufferSize = min(readBufferSize + RX_DEFAULT_BUFFER_SIZE, RX_MAX_BUFFER_SIZE + 1);
            readBuffer = (char *)realloc(readBuffer, readBufferSize);
        }

        // read character
        if(atSerial.readBytes(&c, 1) == 0) {
            break;
        }
        
        // exclude leading <cr><lf>
        if(received == 0 && (c == '\r' || c == '\n')) {
            continue;
        }

        readBuffer[received] = c;
        ++received;


        // stop reading, when <cr><ln> received
        if(received > 2 && readBuffer[received - 2] == '\r' && readBuffer[received - 1] == '\n') {
            received -= 2;
            // put trailing zero
            readBuffer[received - 1] == '\0';
            break;
        }
    }
    
    return received > 0 ? readBuffer : nullptr;
}

void SIM800::writeMessage(const char *message) {
    atSerial.print(message);
    atSerial.write('\r');
}