#include <list>
#include <regex>
#include <iomanip>
#include <ArduinoJson.h>

#include "value_set.h"
#include "logger.h"

using namespace std;

ValueSet::ValueSet() :
timestamp{0}
{}

bool ValueSet::fromJsonString(const char *strPtr, ValueSet &vs)
{
    return fromJsonString(string(strPtr), vs);
}

bool ValueSet::fromJsonString(const string str, ValueSet &vs)
{
    bool res = true;
    DynamicJsonDocument doc(str.length() * 2);
    deserializeJson(doc, str);
    if(!doc.containsKey("timestamp") || !doc.containsKey("sensors")) {
        return false;
    }

    vs.setTimestamp(doc["timestamp"]);
    vs.values.clear();
    for(auto sensor : doc["sensors"].as<JsonArray>()) {
        if(sensor.containsKey("name") && sensor.containsKey("value") && sensor.containsKey("battery") && sensor.containsKey("rssi")) {
            SensorData data;
            data.name = sensor["name"].as<string>();
            data.value = sensor["value"];
            data.battery = sensor["battery"];
            data.rssi = sensor["rssi"];
            vs.addSensorData(data);
        } else {
            res = false;
        }
    }

    return res;
}

void ValueSet::setTimestamp(const time_t tm)
{
    timestamp = tm;
}

time_t ValueSet::getTimestamp()
{
    return timestamp;
}

void ValueSet::addSensorData(SensorData data)
{
    values.push_back(data);
}

bool ValueSet::getSensorData(SensorData &data)
{
    for(auto entry : values) {
        if(entry.name == data.name) {
            data.value = entry.value;
            data.battery = entry.battery;
            data.rssi = entry.rssi;
            return true;
        }
    }

    return false;
}

string ValueSet::toJsonString()
{
    stringstream ss;
    ss << "{\"timestamp\":" << timestamp << ",\"sensors\":[";
    bool nr = 0;
    for(const auto data : values) {
        if(nr > 0) {
            ss << ",";
        };
        
        ss << "{\"name\":\"" << data.name << "\",";
        ss << "\"value\":" << fixed << setprecision(2) << data.value << ",";
        ss << "\"battery\":" << data.battery << ",";
        ss << "\"rssi\":" << data.rssi << "}";
        nr++;
    }

    ss << "]}";

    return ss.str();
}
