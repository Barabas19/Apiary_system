#ifndef LOGGER_H_
#define LOGGER_H_

#include <Arduino.h>

#define DEFAULT_MESSAGE_LENGTH 128

#ifndef LOG_LEVEL
#define LOG_LEVEL               5
#endif

#if LOG_LEVEL < 1
#define LOG_E(format, ...)
#else
#define LOG_E(format, ...)              Esp32Logger::pritnfToLog(FORMAT_LOG(E,format),##__VA_ARGS__)
#endif
#if LOG_LEVEL < 2
#define LOG_W(format, ...)
#else
#define LOG_W(format, ...)              Esp32Logger::pritnfToLog(FORMAT_LOG(W,format),##__VA_ARGS__)
#endif
#if LOG_LEVEL < 3
#define LOG_I(format, ...)
#else
#define LOG_I(format, ...)              Esp32Logger::pritnfToLog(FORMAT_LOG(I,format),##__VA_ARGS__)
#endif
#if LOG_LEVEL < 4
#define LOG_D(format, ...)
#else
#define LOG_D(format, ...)              Esp32Logger::pritnfToLog(FORMAT_LOG(D,format),##__VA_ARGS__)
#endif
#if LOG_LEVEL < 5
#define LOG_V(format, ...)
#else
#define LOG_V(format, ...)              Esp32Logger::pritnfToLog(FORMAT_LOG(V,format),##__VA_ARGS__)
#endif

#define FORMAT_LOG(letter,format) "["#letter"][%s:%u] %s(): " format "\r\n", Esp32Logger::getFileName(__FILE__), __LINE__, __FUNCTION__

namespace Esp32Logger {
    int pritnfToLog(const char *format, ...);
    const char * IRAM_ATTR getFileName(const char *filePath);
}

#endif // LOGGER_H_
