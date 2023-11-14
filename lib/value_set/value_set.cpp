#include "value_set.h"

ValueSet::ValueSet() :
postStrPtr{nullptr}
{}

ValueSet::~ValueSet()
{
    free(postStrPtr);
}

ValueSet::ValueSet(JsonObject &json) :
postStrPtr{nullptr}
{
    if(json.containsKey("timestamp") && json.containsKey("values")) {
        jsonObject = JsonObject(json);
    }
}

void ValueSet::setTimestamp(time_t tm)
{
    jsonObject["timestamp"] = tm;
}

void ValueSet::addValue(const char *name, float value)
{
    JsonObject newValue;
    newValue[name] = value;
    jsonObject["values"].add(newValue);
}

JsonObject ValueSet::getJsonObject()
{
    return jsonObject;
}

const char * ValueSet::getPostString()
{
    // verification
    if(!jsonObject.containsKey("timestamp") || !jsonObject.containsKey("values")) {
        return nullptr;
    }

    // release previously allocated memory
    if(postStrPtr) {
        free(postStrPtr);
    }

    // allocate enough memory for string
    postStrPtr = (char *)calloc(sizeof(jsonObject), 1);
    sprintf(postStrPtr, "timestamp=%d", jsonObject["timestamp"].as<long>());
    for(JsonPair kv : jsonObject) {
        sprintf(postStrPtr, "%s&%s=%.1f", postStrPtr, kv.key().c_str(), kv.value().as<float>());
    }
    
    return postStrPtr;
}
