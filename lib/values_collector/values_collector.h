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
    /** Starts the BLE scanning and collecting data from the found devices.
     * @nextWindowdt - predefined time of the next collecting window
    */
    static void run(struct tm &nextWindowdt);

    /** Returns collected values
     * @returns - the value set
    */
    static ValueSet getCollectedValues();

    /** Says, if the collecting is finished
     * @returns - true, if collecting is finished
    */
    static inline bool isFinished() { return finished; }

    /** Calculates time until next collecting window
     * @nextWindowdt - predefined time of the next collecting window
     * @returns - calculated time in seconds
    */
    static time_t calculateSleepTime(struct tm &nextWindowdt);

private:
    static void collectDeviceData(BLEAdvertisedDevice *device, BLEClient *bleClient);
    static BLEAdvertisedDevice *foundDevice;
    static std::map<BLEAddress, SensorData> processedDevices; // the devices that have been already processed (+ their data)
    static std::list<BLEAddress> excludedDevices; // the devices that don't have required services or characteristics
    static bool finished;
    static struct tm &nextWindowOpenTime;

    friend class AdvertisedCallbacks;
};

#endif // VALUES_COLLECTOR_H_