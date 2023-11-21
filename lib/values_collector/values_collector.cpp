#include <time.h>

#include "values_collector.h"
#include "bluetooth_specs.h"
#include "logger.h"
#include "project_config.h"

// based on https://electropeak.com/learn/esp32-bluetooth-low-energy-ble-on-arduino-ide-tutorial/

using namespace std;

void ValuesCollector::run() {
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
    while(millis() < endTime) {
        BLEDevice::getScan()->start((endTime - millis()) / 1000);
        if(foundDevice) {
            collectSlaveData(foundDevice, client);
            foundDevice = nullptr;
        }
    }

    finished = true;
}

ValueSet ValuesCollector::getCollectedValues()
{
    time_t now;
    time(&now);
    values.setTimestamp(now);
    for(auto kv : processedSlaves) {
        values.addValue(kv.second.name.c_str(), kv.second.sensorValue);
    }

    // TODO: provide all the collected data (including battery level, BLE address, RSSI)

    return values;
}

void ValuesCollector::collectSlaveData(BLEAdvertisedDevice *device, BLEClient *bleClient)
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
        excludedSlaves.push_back(device->getAddress());
        return;
    }

    SlaveData data;
    data.name = device->getName();
    data.bleAddr = device->getAddress().toString();
    data.batteryLevel = batteryChar->readUInt8();
    data.sensorValue = sensorValueChar->readFloat();
    data.rssi = device->getRSSI();
    processedSlaves[device->getAddress()] = data;
    sleepTimeChar->writeValue(to_string(calculateSleepTime()));
    LOG_I("BLE device '%s' successfully processed.", device->getName().c_str());
}

uint32_t ValuesCollector::calculateSleepTime()
{
    return 0;
}

class AdvertisedCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        LOG_I("Found BLE device '%s'.", advertisedDevice.toString().c_str());
        // Verify if the found device has not been processed yet
        // and it advertises required services
        if(ValuesCollector::processedSlaves.count(advertisedDevice.getAddress()) == 0 &&
        std::find(ValuesCollector::excludedSlaves.begin(), ValuesCollector::excludedSlaves.end(), advertisedDevice.getAddress()) == ValuesCollector::excludedSlaves.end() &&
        advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(BLEUUID(BATTERY_SERVICE_UUID)) &&
        advertisedDevice.isAdvertisingService(BLEUUID(SENSOR_VALUE_SERVICE_UUID)) &&
        advertisedDevice.isAdvertisingService(BLEUUID(SLEEP_TIME_SERVICE_UUID)) ) {
            BLEDevice::getScan()->stop();
            ValuesCollector::foundDevice = new BLEAdvertisedDevice(advertisedDevice);
        }
    }
};
