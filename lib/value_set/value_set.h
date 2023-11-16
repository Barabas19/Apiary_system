#ifndef VALUE_SET_H
#define VALUE_SET_H

#include <Arduino.h>
#include <string>
#include <map>

// structure of json is described in
// docs/Sensor_values_example.json

class ValueSet {
public:
    ValueSet();

    /** Create new value set based on string in format 'timestamp=123&name1=1.2&name2=2.3&...'
     * @strPtr - name/values should be separated with '&' or ';'
    */
    static ValueSet fromString(const char *strPtr);
    static ValueSet fromString(const std::string srcStr);

    /** Set time stamp of value set
     * @tm - epoch time
    */
    void setTimestamp(const time_t tm);

    /** Get time stamp of value set
     * @returns - epoch time
    */
    time_t  getTimestamp();

    /** Add a pair of name and value to the value set
     * @name - value name
     * @value - floating-point value
    */
    void addValue(const char *name, float value);

    /** Get value of the specified name
     * @returns - value
    */
    float getValue(const char *name);

    /** Convert value set to string prepared for POST request
     * @return - string for POST request
    */
    std::string toString();

private:
    time_t timestamp;
    std::map<std::string, float> values;
};

#endif // VALUE_SET_H