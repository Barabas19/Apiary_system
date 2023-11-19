#ifndef VALUES_COLLECTOR_H_
#define VALUES_COLLECTOR_H_

#include "value_set.h"

class ValuesCollector {
public:
    static void run(uint32_t duration);
    static bool startCollecting();
    static bool stopCollecting();
    static ValueSet getCollectedValues();
private:
    static ValueSet values;
    static bool collectingWindowOpen;
    static bool device_connected;

    friend class MyServerCallbacks;
};

#endif // VALUES_COLLECTOR_H_