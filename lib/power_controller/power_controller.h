#ifndef _POWER_CONTROLLER_H_
#define _POWER_CONTROLLER_H_

#include <time.h>
#include <functional>
#include <list>

using namespace std;

#define MINIMUM_SLEEP_TIME_S 60

class PowerController {
public:
    static void init();
    static void registerGetBurstCallback(function<int()> callback);
    static void registerPowerOffCallback(function<bool()> callback);
    static bool isStartedInConfigMode();
    static time_t getNextWakeTime();
    static void deepSleep();

private:
    static function<int()> getBurstCallback;
    static list<function<bool()>> powerOffCallbacks;
};

#endif // _POWER_CONTROLLER_H_