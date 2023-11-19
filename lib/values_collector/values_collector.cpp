#include <BLEDevice.h>

#include "values_collector.h"
#include "logger.h"

void ValuesCollector::run(uint32_t duration) {
    while(true) {
        while(!collectingWindowOpen) {
            delay(100);
        }

        LOG_I("Start collecting data from sensors.");
        BLEDevice::init("ApiaryMaster");
        BLEServer *pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());
    }
}

bool ValuesCollector::startCollecting()
{
    return false;
}

bool ValuesCollector::stopCollecting()
{
    return false;
}

ValueSet ValuesCollector::getCollectedValues()
{
    return ValueSet();
}

class MyServerCallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* pServer) {
        ValuesCollector::device_connected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        ValuesCollector::device_connected = false;
    };
};
