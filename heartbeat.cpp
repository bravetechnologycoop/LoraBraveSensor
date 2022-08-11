#include <Arduino.h>
#include <ArduinoJson.h>
#include "heartbeat.h"
#include "lora.h"
#include "systemTimers.h"
#include "flashAddresses.h"

static int HEARTBEAT_INTERVAL = 300000;
static int heartbeatTimer = HEARTBEAT_INTERVAL;
static int lastHeartbeatHandleTime = 0;

unsigned int getHeartbeatRemainingDuration()
{
    heartbeatTimer -= millis() - lastHeartbeatHandleTime;
    lastHeartbeatHandleTime = millis();
    Serial.printf("Heartbeat remaining duration: %i\r\n", heartbeatTimer);

    if (heartbeatTimer <= 0)
    {
        heartbeatTimer = HEARTBEAT_INTERVAL;
        DynamicJsonDocument doc(1024);
        doc["alertType"] = "heartbeat";
        lora::sendUplink(doc);
        Serial.println("heartbeat");
    }
    return heartbeatTimer;
}

unsigned int getHeartbeatInterval()
{
    return HEARTBEAT_INTERVAL;
}