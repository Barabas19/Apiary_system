#ifndef VALUES_COLLECTOR_H_
#define VALUES_COLLECTOR_H_

#include <BLEDevice.h>
#include <string>
#include <map>
#include <list>

#include "value_set.h"

struct SlaveData {
    std::string name;
    std::string bleAddr;
    uint8_t batteryLevel;
    float sensorValue;
    int rssi;
};

class ValuesCollector {
public:
    static void run();
    static ValueSet getCollectedValues();
    static inline bool isFinished() { return finished; }
private:
    static void collectSlaveData(BLEAdvertisedDevice *device, BLEClient *bleClient);
    static uint32_t calculateSleepTime();
    static ValueSet values;
    static BLEAdvertisedDevice *foundDevice;
    static std::map<BLEAddress, SlaveData> processedSlaves;
    static std::list<BLEAddress> excludedSlaves;
    static bool finished;

    friend class AdvertisedCallbacks;
};

#endif // VALUES_COLLECTOR_H_