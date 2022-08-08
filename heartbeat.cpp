#include <Arduino.h>
#include "heartbeat.h"
#include "OTAA.h"
#include "systemTimers.h"
#include "flashAddresses.h"

static unsigned int HEARTBEAT_INTERVAL = 0;

void setupHeartbeat()
{
    getHeartbeatInterval();
    api.system.timer.create((RAK_TIMER_ID)HEARTBEAT_TIMER, (RAK_TIMER_HANDLER)heartbeatHandler, RAK_TIMER_PERIODIC);
    api.system.timer.start((RAK_TIMER_ID)HEARTBEAT_TIMER, HEARTBEAT_INTERVAL, (void *)1);
    Serial.printf("Heartbeat interval: %u", HEARTBEAT_INTERVAL);
}

void heartbeatHandler(void)
{
    Serial.println("Heartbeat");
    uplink_routine("Heartbeat");
}

bool setHeartbeatInterval(unsigned int interval)
{
    bool success = api.system.flash.set(HEARTBEAT_INTERVAL_FLASH_ADDRESS, (uint8_t *)&interval, sizeof(HEARTBEAT_INTERVAL));
    setupHeartbeat();
    return success;
}

bool getHeartbeatInterval()
{
    return api.system.flash.get(HEARTBEAT_INTERVAL_FLASH_ADDRESS, (uint8_t *)&HEARTBEAT_INTERVAL, sizeof(HEARTBEAT_INTERVAL));
}