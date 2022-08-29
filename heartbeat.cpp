#include <Arduino.h>
#include <ArduinoJson.h>
#include "heartbeat.h"
#include "lora.h"
#include "systemTimers.h"
#include "flashAddresses.h"
#include "main.h"

const unsigned int MIN_HEARTBEAT_INTERVAL = 5000;      // 5s
const unsigned int MAX_HEARTBEAT_TIMER = 21600000;     // 6h
const unsigned int DEFAULT_HEARTBEAT_INTERVAL = 60000; // 60s

static int heartbeatInterval = DEFAULT_HEARTBEAT_INTERVAL;
static int heartbeatTimer = 0;
static int lastHeartbeatHandleTime = 0;

unsigned int heartbeat::getRemainingDuration()
{
    heartbeatTimer -= millis() - lastHeartbeatHandleTime;
    lastHeartbeatHandleTime = millis();

    if (heartbeatTimer <= 0)
    {
        heartbeatTimer = heartbeatInterval;
        lora::uplinkMessage msg = {.type = lora::HEARTBEAT};
        lora::sendUplink(msg);
        DEBUG_SERIAL_LOG.println("Heartbeat");
    }
    return heartbeatTimer;
}

unsigned int heartbeat::getInterval()
{
    return heartbeatInterval;
}

int heartbeat::setInterval(unsigned int interval)
{
    if (interval >= MIN_HEARTBEAT_INTERVAL && interval <= MAX_HEARTBEAT_TIMER)
    {
        bool success = api.system.flash.set(HEARTBEAT_INTERVAL_FLASH_ADDRESS, (uint8_t *)&interval, sizeof(heartbeatInterval));
        if (success)
        {
            heartbeatInterval = interval;
            DEBUG_SERIAL_LOG.printf("Heartbeat interval set to %u\r\n", heartbeatInterval);
            return heartbeatInterval;
        }
    }
    DEBUG_SERIAL_LOG.printf("Failed to set heartbeat interval to %u\r\n", interval);
    return -1;
}

void heartbeat::resetTimers()
{
    heartbeat::setInterval(DEFAULT_HEARTBEAT_INTERVAL);
}

void heartbeat::setupHeartbeat()
{
    if (api.system.flash.get(HEARTBEAT_INTERVAL_FLASH_ADDRESS, (uint8_t *)&heartbeatInterval, sizeof(heartbeatInterval)))
    {
        DEBUG_SERIAL_LOG.printf("Heartbeat interval set to %u\r\n", heartbeatInterval);
    }
    else
    {
        DEBUG_SERIAL_LOG.println("ERROR: could not read heartbeat interval from flash");
        lora::uplinkMessage msg = {.type = lora::EEPROM_ERROR};
        lora::sendUplink(msg);
    }
}