#ifndef VALUE_SET_H
#define VALUE_SET_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

// structure of json is described in
// docs/Sensor_values_example.json

class ValueSet {
public:
    ValueSet();
    ~ValueSet();

    /** Create new value set based on json object
     * @json - json object
    */
    ValueSet(JsonObject &json);

    /** Set time stamp of value set
     * @tm - timestamp
    */
    void setTimestamp(time_t tm);

    /** Add a pair of name and value to the value set
     * @name - value name
     * @value - floating-point value
    */
    void addValue(const char *name, float value);

    /** Create a json object
     * @returns - json object
    */
    JsonObject getJsonObject();

    /** Convert value set to string prepared for POST request
     * @return - pointer to string for POST request
    */
    const char * getPostString();

private:
    JsonObject jsonObject;
    char *postStrPtr;
};

#endif // VALUE_SET_H