#include <list>
#include <regex>
#include <iomanip>

#include "value_set.h"
#include "logger.h"

using namespace std;

ValueSet::ValueSet() :
timestamp{0}
{}

ValueSet ValueSet::fromString(const char *strPtr)
{
    return fromString(string(strPtr));
}

ValueSet ValueSet::fromString(const string str)
{
    ValueSet vs;
    string srcStr = str;
    regex rgx("[a-z0-9]+=[0-9\\.]+");
    string match, name, value;
    
    for(smatch sm; regex_search(srcStr, sm, rgx); ) {
        match = sm.str();
        name = match.substr(0, match.find('='));
        value = match.substr(match.find('=') + 1);
        srcStr = sm.suffix();

        if(name == "timestamp") {
            vs.timestamp = static_cast<time_t>(stol(value));
        } else {
            vs.values[name] = stof(value);
        }
    }

    return vs;
}

void ValueSet::setTimestamp(const time_t tm)
{
    timestamp = tm;
}

time_t ValueSet::getTimestamp()
{
    return timestamp;
}

void ValueSet::addValue(const char *name, float value)
{
    values[name] = value;
}

float ValueSet::getValue(const char *name)
{
    return values[name];
}

string ValueSet::toString()
{
    stringstream ss;
    ss << "timestamp=" << timestamp;
    for(const auto kv : values) {
        ss << "&" << kv.first << "=" << fixed << setprecision(2) << kv.second;
    }

    return ss.str();
}
