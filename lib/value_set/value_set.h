#ifndef VALUE_SET_H
#define VALUE_SET_H

#include <Arduino.h>
#include <string>
#include <list>

#include "sensor_data_struct.h"

// structure of json is described in
// docs/Sensor_values_example.json

class ValueSet {
public:
    ValueSet();

    /** Deserialize ValueSet from JSON string
     * @strPtr - string in JSON format
     * @vs - target ValueSet
     * @returns - true, if successfully deserialized
    */
    static bool fromJsonString(const char *strPtr, ValueSet &vs);
    static bool fromJsonString(const std::string srcStr, ValueSet &vs);

    /** Set time stamp of value set
     * @tm - epoch time
    */
    void setTimestamp(const time_t tm);

    /** Get time stamp of value set
     * @returns - epoch time
    */
    time_t  getTimestamp();

    /** Add sensor data to value set
     * @data - sensor data
    */
    void addSensorData(SensorData data);

    /** Get sensor data. data["name"] must be specified before call.
     * @data - sensor data
     * @returns - true, if found
    */
    bool getSensorData(SensorData &data);

    /** Convert value set to string in JSON format
     * @return - JSON string
    */
    std::string toJsonString();

private:
    time_t timestamp;
    std::list<SensorData> values;
};

#endif // VALUE_SET_H