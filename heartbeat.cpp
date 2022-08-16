#include <Arduino.h>
#include <ArduinoJson.h>
#include "heartbeat.h"
#include "lora.h"
#include "systemTimers.h"
#include "flashAddresses.h"
#include "main.h"

static int HEARTBEAT_INTERVAL = 300000;
static int heartbeatTimer = 0; 
static int lastHeartbeatHandleTime = 0;

unsigned int heartbeat::getRemainingDuration()
{
    heartbeatTimer -= millis() - lastHeartbeatHandleTime;
    lastHeartbeatHandleTime = millis();

    if (heartbeatTimer <= 0)
    {
        heartbeatTimer = HEARTBEAT_INTERVAL;
        lora::uplinkMessage msg = {.alertType = lora::uplinkMessage::HEARTBEAT}; 
        lora::sendUplink(msg);
        DEBUG_SERIAL_LOG.println("Heartbeat");
    }
    return heartbeatTimer;
}

unsigned int heartbeat::getInterval()
{
    return HEARTBEAT_INTERVAL;
}