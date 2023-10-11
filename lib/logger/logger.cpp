#include <logger.h>

namespace Esp32Logger {
    int pritnfToLog(const char *format, ...) {
        static char locBuff[DEFAULT_MESSAGE_LENGTH];
        char *buffPtr = locBuff;
        va_list arg;
        va_start(arg, format);
        int len = vsnprintf(NULL, 0, format, arg);
        if(len >= sizeof(locBuff)) {
            buffPtr = (char *)malloc(len + 1);
            if(buffPtr == nullptr) {
                return 0;
            }
        }

        vsnprintf(buffPtr, len + 1, format, arg);
        ets_printf("%s", buffPtr);
        va_end(arg);

        if(buffPtr != locBuff) {
            free(buffPtr);
        }

        return len;
    }

    const char * IRAM_ATTR getFileName(const char *filePath) {
        size_t i = 0;
        size_t pos = 0;
        char *p = (char *)filePath;
        while(*p) {
            i++;
            if(*p == '/' || *p == '\\') {
                pos = i;
            }
            p++;
        }

        return filePath + pos;
    }
}