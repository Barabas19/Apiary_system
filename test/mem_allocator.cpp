#include <stdlib.h>
#include <iostream>
#include <cstring>

#define DEFAULT_SIZE 16
#define MAX_SIZE 65535

using namespace std;

char *buffer = nullptr;

char* readMessageToBuffer(const char *message, int &size) {
    int obtained = 0;
    char c;
    memset(buffer, 0, size);

    while(true) {
        if(obtained >= size - 1) {
            int newSize = size + DEFAULT_SIZE;
            // cout << "size = " << size << ", new size = " << newSize << endl;
            buffer = (char *)realloc(buffer, newSize);
            memset(&(buffer[size]), 0, newSize - size);
            size = newSize;
            cout << "Reallocated to " << size << "B" << endl;
        }

        c = message[obtained];
        if(c == '\0' || c == '\n') {
            break;
        }

        if(c == '\r') {
            continue;
        }

        buffer[obtained++] = c;
    }

    return obtained > 0 ? buffer : nullptr;
}

int main() {
    int size = DEFAULT_SIZE;
    buffer = (char *)malloc(size);
    const char *messages[] = {
        "Hello",
        "Pointer to the block of memory to fill.",
        "Sets the first num bytes of the block of memory pointed by ptr to the specified value (interpreted as an unsigned char).",
        "Two slash signs indicate that the rest of the line is a comment inserted by the programmer but which has no effect on the behavior of the program. Programmers use them to include short explanations or observations concerning the code or program. In this case, it is a brief introductory description of the program."
    };

    for(int i = 0; i < sizeof(messages) / sizeof(const char *); i++) {
        cout << "***** Reading message " << i + 1 << " *****" << endl;
        char *mess = readMessageToBuffer(messages[i], size);
        if(!mess) {
            cout << "Nothing read" << endl;
        } else {
            cout << "Buffer: " << mess << endl;
            free(mess);
        }
    }

    
}