#ifndef VALUE_SET_H
#define VALUE_SET_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <string>

// structure of json is described in
// docs/Sensor_values_example.json

class ValueSet {
public:
    ValueSet();
    ValueSet(JsonObject &json);
    void setTimestamp(time_t tm);
    void addValue(const char *name, float value);
    JsonObject getJsonObject();
    std::string getPostString();

private:
    time_t timestamp;
    std::map<std::string, float> values;
};

#endif // VALUE_SET_H