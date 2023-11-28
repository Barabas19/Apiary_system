#ifndef _SENSOR_DATA_STRUCT_H_
#define _SENSOR_DATA_STRUCT_H_

#include <string>

struct SensorData
{
    std::string name;
    float value;
    uint8_t battery;
    int rssi;
};

#endif // _SENSOR_DATA_STRUCT_H_