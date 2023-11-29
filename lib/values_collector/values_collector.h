#ifndef VALUES_COLLECTOR_H_
#define VALUES_COLLECTOR_H_

#include <BLEDevice.h>
#include <string>
#include <map>
#include <list>

#include "value_set.h"
#include "sensor_data_struct.h"

#define DEFAULT_SLEEP_TIME_S 60

class ValuesCollector {
public:
    static void run(struct tm &nextWindowdt, bool &dtValid);
    static ValueSet getCollectedValues();
    static inline bool isFinished() { return finished; }
private:
    static void collectDeviceData(BLEAdvertisedDevice *device, BLEClient *bleClient);
    static uint32_t calculateSleepTime();
    static BLEAdvertisedDevice *foundDevice;
    static std::map<BLEAddress, SensorData> processedDevices; // the devices that have been already processed (+ their data)
    static std::list<BLEAddress> excludedDevices; // the devices that don't have required services or characteristics
    static bool finished;
    static struct tm &nextWindowOpenTime;
    static bool &nextWindowOpenTimeValid;

    friend class AdvertisedCallbacks;
};

#endif // VALUES_COLLECTOR_H_