#include <time.h>

#include "values_collector.h"
#include "bluetooth_specs.h"
#include "logger.h"
#include "project_config.h"

// based on https://electropeak.com/learn/esp32-bluetooth-low-energy-ble-on-arduino-ide-tutorial/

using namespace std;

void ValuesCollector::run(struct tm &nextWindowdt, bool &dtValid) {
    nextWindowOpenTime = nextWindowdt;
    nextWindowOpenTimeValid = dtValid;

    finished = false;
    BLEDevice::init("ApiaryMaster");
    BLEScan *scan = BLEDevice::getScan();
    BLEClient *client = BLEDevice::createClient();
    scan->setAdvertisedDeviceCallbacks(new AdvertisedCallbacks());
    scan->setActiveScan(true);
    scan->setInterval(1000);
    scan->setWindow(100);
    foundDevice = nullptr;

    auto startTime = millis();
    auto endTime = startTime + VALUES_COLLECTING_DURATION_S * 1000;
    bool scanContinues = false;
    while(millis() < endTime) {
        BLEDevice::getScan()->start((endTime - millis()) / 1000 + 1, scanContinues);
        scanContinues = true;
        if(foundDevice) {
            collectDeviceData(foundDevice, client);
            foundDevice = nullptr;
        }
    }

    finished = true;
}

ValueSet ValuesCollector::getCollectedValues()
{
    time_t now;
    time(&now);
    ValueSet values;
    values.setTimestamp(now);
    for(auto kv : processedDevices) {
        values.addSensorData(kv.second);
    }

    return values;
}

void ValuesCollector::collectDeviceData(BLEAdvertisedDevice *device, BLEClient *bleClient)
{
    bleClient->connect(device);
    auto batterySrv = bleClient->getService(BLEUUID(BATTERY_SERVICE_UUID));
    auto sensorValueSrv = bleClient->getService(BLEUUID(SENSOR_VALUE_SERVICE_UUID));
    auto sleepTimeSrv = bleClient->getService(BLEUUID(SLEEP_TIME_SERVICE_UUID));

    auto batteryChar = batterySrv->getCharacteristic(BLEUUID(BATTERY_CHARACTERISTIC_UUID));
    auto sensorValueChar = sensorValueSrv->getCharacteristic(BLEUUID(SENSOR_VALUE_CHARACTERISTIC_UUID));
    auto sleepTimeChar = sleepTimeSrv->getCharacteristic(BLEUUID(SLEEP_TIME_CHARACTERISTIC_UUID));
    if(!batteryChar || ! sensorValueChar || ! sleepTimeChar) {
        LOG_W("BLE device '%s' cannot be processed - required characteristic not found.", device->getName().c_str());
        excludedDevices.push_back(device->getAddress());
        return;
    }

    SensorData data;
    data.name = device->getName();
    data.battery = batteryChar->readUInt8();
    data.value = sensorValueChar->readFloat();
    data.rssi = device->getRSSI();
    processedDevices[device->getAddress()] = data;
    sleepTimeChar->writeValue(to_string(calculateSleepTime()));
    LOG_I("BLE device '%s' successfully processed.", device->getName().c_str());
}

uint32_t ValuesCollector::calculateSleepTime()
{
    if(!nextWindowOpenTimeValid) {
        LOG_W("Next collecting window time is not set, default sleep time will be used - %ds.", DEFAULT_SLEEP_TIME_S);
        return DEFAULT_SLEEP_TIME_S;
    }

    time_t nextEpoch = mktime(&nextWindowOpenTime);
    if(nextEpoch = -1) {
        LOG_E("Next collecting window time is invalid, default sleep time will be used - %ds.", DEFAULT_SLEEP_TIME_S);
        return DEFAULT_SLEEP_TIME_S;
    }

    time_t now;
    time(&now);
    if(now < 3600) {
        LOG_E("System time is not actualized, default sleep time will be used - %ds.", DEFAULT_SLEEP_TIME_S);
        return DEFAULT_SLEEP_TIME_S;
    }

    LOG_V("Calculated sleep time %ds.", nextEpoch - now);
    return nextEpoch - now;
}

class AdvertisedCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        LOG_I("Found BLE device '%s'.", advertisedDevice.toString().c_str());
        // Verify if the found device has not been processed yet
        // and it advertises required services
        if(ValuesCollector::processedDevices.count(advertisedDevice.getAddress()) == 0 &&
        std::find(ValuesCollector::excludedDevices.begin(), ValuesCollector::excludedDevices.end(), advertisedDevice.getAddress()) == ValuesCollector::excludedDevices.end() &&
        advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(BLEUUID(BATTERY_SERVICE_UUID)) &&
        advertisedDevice.isAdvertisingService(BLEUUID(SENSOR_VALUE_SERVICE_UUID)) &&
        advertisedDevice.isAdvertisingService(BLEUUID(SLEEP_TIME_SERVICE_UUID)) ) {
            BLEDevice::getScan()->stop();
            ValuesCollector::foundDevice = new BLEAdvertisedDevice(advertisedDevice);
        }
    }
};
