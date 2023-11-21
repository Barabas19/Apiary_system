#ifndef BLUETOOTH_SPECS_H_
#define BLUETOOTH_SPECS_H_

#include <BLEDevice.h>


// The following UUIDs should or can be used for data exchange between bluetooth devices.

// https://www.bluetooth.com/specifications/assigned-numbers/
#define BATTERY_SERVICE_UUID                ((uint32_t)0x180F)
#define BATTERY_CHARACTERISTIC_UUID         ((uint32_t)0x2A19)
#define SENSOR_VALUE_SERVICE_UUID           "f6d4c4ce-48a1-11ec-81d3-0242ac130003"
#define SENSOR_VALUE_CHARACTERISTIC_UUID    "002ebd2c-48a2-11ec-81d3-0242ac130003"
#define SLEEP_TIME_SERVICE_UUID             "1266c68c-4871-11ec-81d3-0242ac130003"
#define SLEEP_TIME_CHARACTERISTIC_UUID      "23a1e2ba-4871-11ec-81d3-0242ac130003"
#define CALIBRATION_SERVICE_UUID            "138b6a0e-496b-11ec-81d3-0242ac130003"
#define CALIBRATION_CHARACTERISTIC_UUID     "24e90e00-496b-11ec-81d3-0242ac130003"

#endif // BLUETOOTH_SPECS_H_