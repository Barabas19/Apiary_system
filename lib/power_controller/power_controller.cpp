#include "power_controller.h"
#include "ip5306.h"
#include "logger.h"

using namespace std;

void PowerController::init()
{
    set_ip5306();
}

void PowerController::registerGetBurstCallback(function<int()> callback)
{
    getBurstCallback = callback;
}

void PowerController::registerPowerOffCallback(function<bool()> callback)
{
    powerOffCallbacks.push_back(callback);
}

bool PowerController::isStartedInConfigMode()
{
    return false;
}

time_t PowerController::getNextWakeTime()
{
    int burst = getBurstCallback();
    time_t now = time(NULL);
    time_t nextWakeUp = (now / burst + 1) * burst;
    if(nextWakeUp - now < MINIMUM_SLEEP_TIME_S) {
        nextWakeUp += burst;
    }
    
    return nextWakeUp;
}

void PowerController::deepSleep()
{
    for(auto callback : powerOffCallbacks) {
        callback();
    }

    time_t nextWake = getNextWakeTime();
    time_t now = time(NULL);
    uint64_t sleepUs = (nextWake - now) * 1000000LL;
    if(esp_sleep_enable_timer_wakeup(sleepUs) != ESP_OK) {
        LOG_E("Failed to enable timer_wakeup.");
    }

    LOG_I("Enter deep sleep for %ds", nextWake - now);
    esp_deep_sleep_start();
}

function<int()> PowerController::getBurstCallback;
list<function<bool()>> PowerController::powerOffCallbacks;