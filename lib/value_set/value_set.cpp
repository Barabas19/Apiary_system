#include "value_set.h"

ValueSet::ValueSet() :
timestamp{0}
{}

ValueSet::ValueSet(JsonObject &json) :
timestamp{0}
{
    if(json.containsKey("timestamp")) {
        timestamp = json["timestamp"];
    }

    if(json.containsKey("values")) {
        JsonObject jsonValues = json["values"].as<JsonObject>();
        for(JsonPair kv : jsonValues) {
            values[std::string(kv.key().c_str())] = kv.value().as<float>();
        }
    }
}

void ValueSet::setTimestamp(time_t tm)
{
    timestamp = tm;
}

void ValueSet::addValue(const char *name, float value)
{
    values[std::string(name)] = value;
}

JsonObject ValueSet::getJsonObject()
{
    JsonObject json;
    json["timestamp"] = timestamp;
    json.createNestedObject("values");
    for(const auto &kv : values) {
        json["values"][kv.first] = kv.second;
    }

    return json;
}

std::string ValueSet::getPostString()
{
    std::string postStr = "timestamp=" + timestamp;
    for(const auto &kv : values) {
        postStr += "&";
        postStr += kv.first;
        postStr += kv.second;
    }

    return postStr;
}
