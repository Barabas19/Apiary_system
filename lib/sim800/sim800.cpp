#include <sim800.h>

SIM800::SIM800(HardwareSerial atSerial, gpio_num_t txPin, gpio_num_t rxPin, gpio_num_t powerPin, gpio_num_t pwrKeyPin, gpio_num_t rstPin, gpio_num_t dtrPin) :
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

char* SIM800::readMessage() {
    int received = 0;
    for(int buffered = atSerial.available(); buffered > 0; buffered = atSerial.available()) {
        // extend buffer size, if necessary
        if(received + buffered >= readBufferSize - 1) {
            int newSize = received + buffered + 1;
            readBuffer = (char *)realloc(readBuffer, newSize);
        }

        // read buffered bytes
        int readBuffered = atSerial.readBytes(readBuffer + received, buffered);
        if(readBuffered == 0) {
            break;
        }

        received += readBuffered;

        // check, if <cr><ln> received at the end of message
        if(received > 2 && readBuffer[received - 3] != '\r' && readBuffer[received - 3] != '\n'
                        && readBuffer[received - 2] == '\r' && readBuffer[received - 1] == '\n') {
            received -= 1;
            break;
        }
    }

    if(received > 0) {
        // put trailing zero
        readBuffer[received - 1] == '\0';
        // exclude leading <cr><ln>
        for(int i = 0; i < received; i++) {
            if(readBuffer[i] != '\r' && readBuffer[i] != '\n') {
                return &readBuffer[i];
            }
        }
    }

    return nullptr;
}

void SIM800::writeMessage(const char *message) {
    atSerial.print(message);
    // atSerial.write('\r');
}