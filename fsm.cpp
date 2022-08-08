#include <Arduino.h>
#include <ArduinoJson.h>
#include "fsm.h"
#include "sensors.h"
#include "lora.h"
#include "flashAddresses.h"

stateHandler_t stateHandler = state0_idle;
unsigned int lastStateHandleTime = millis();

static unsigned int COUNTDOWN_TIMER = 0;
static unsigned int DURATION_TIMER = 0;
static unsigned int STILLNESS_TIMER = 0;

int state0_idle_timer = 0;
int state1_15sCountdown_timer = 0;
int state2_duration_timer = 0;
int state3_stillness_timer = 0;

void setupFSM()
{
    getFlash();
    Serial.printf("Timers constants: countdown: %u, duration: %u, stillness: %u\r\n", COUNTDOWN_TIMER, DURATION_TIMER, STILLNESS_TIMER);
}

bool getFlash()
{
    return api.system.flash.get(0, (uint8_t *)&COUNTDOWN_TIMER, sizeof(COUNTDOWN_TIMER)) &&
           api.system.flash.get(4, (uint8_t *)&DURATION_TIMER, sizeof(DURATION_TIMER)) &&
           api.system.flash.get(8, (uint8_t *)&STILLNESS_TIMER, sizeof(STILLNESS_TIMER));
}

bool setCountdownTimer(unsigned int timer)
{
    bool success = api.system.flash.set(COUNTDOWN_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(COUNTDOWN_TIMER));
    COUNTDOWN_TIMER = timer;
    return success;
}

bool setDurationTimer(unsigned int timer)
{
    bool success = api.system.flash.set(DURATION_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(DURATION_TIMER));
    DURATION_TIMER = timer;
    return success;
}

bool setStillnessTimer(unsigned int timer)
{
    bool success = api.system.flash.set(STILLNESS_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(STILLNESS_TIMER));
    STILLNESS_TIMER = timer;
    return success;
}

unsigned int getCountdownTimer()
{
    return COUNTDOWN_TIMER;
}

unsigned int getDurationTimer()
{
    return DURATION_TIMER;
}

unsigned int getStillnessTimer()
{
    return STILLNESS_TIMER;
}

int state0_idle(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state0_idle_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (motionSensor.isThereMotion() && !doorSensor.isDoorOpen())
    {
        stateHandler = state1_15sCountdown;
        Serial.println("state 0 -> state 1: motion detected");

        state1_15sCountdown_timer = COUNTDOWN_TIMER;
        return state1_15sCountdown_timer;
    }
    return 0;
}

int state1_15sCountdown(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state1_15sCountdown_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (!motionSensor.isThereMotion() || doorSensor.isDoorOpen())
    {
        stateHandler = state0_idle;
        Serial.println("state 1 -> state 0: no motion, door closed");
        return 1;
    }
    else if (state1_15sCountdown_timer <= 0)
    {
        stateHandler = state2_duration;
        Serial.printf("state 1 -> state 2: motion detected for %u seconds\r\n", COUNTDOWN_TIMER / 1000);

        state2_duration_timer = DURATION_TIMER;
        return state2_duration_timer;
    }
    return state1_15sCountdown_timer;
}

int state2_duration(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state2_duration_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (!motionSensor.isThereMotion())
    {
        stateHandler = state3_stillness;
        Serial.println("state 2 -> state 3: no motion");

        state3_stillness_timer = STILLNESS_TIMER;
        return state3_stillness_timer;
    }
    else if (state2_duration_timer <= 0)
    {
        stateHandler = state0_idle;
        Serial.println("Duration Alert!!");
        DynamicJsonDocument doc(1024); 
        doc["alertType"] = "DurationAlert"; 
        uplink_routine(doc);
        Serial.println("state 2 -> state 0: duration alert");
        return 1;
    }
    else if (doorSensor.isDoorOpen())
    {
        stateHandler = state0_idle;
        Serial.println("state 2 -> state 0: door opened, session over");
        return 1;
    }
    return state2_duration_timer;
}

int state3_stillness(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state2_duration_timer -= millis() - lastStateHandleTime;
    state3_stillness_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (motionSensor.isThereMotion())
    {
        stateHandler = state2_duration;
        Serial.println("state 3 -> state 2: motion detected");
        return state2_duration_timer;
    }
    else if (state3_stillness_timer <= 0)
    {
        stateHandler = state0_idle;
        Serial.println("Stillness Alert!!");
        DynamicJsonDocument doc(1024); 
        doc["alertType"] = "StillnessAlert"; 
        uplink_routine(doc);
        Serial.println("state 3 -> state 0: stillness alert");
        return 1;
    }
    else if (state2_duration_timer <= 0)
    {
        stateHandler = state0_idle;
        Serial.println("Duration Alert!!");
        DynamicJsonDocument doc(1024); 
        doc["alertType"] = "DurationAlert"; 
        uplink_routine(doc);
        Serial.println("state 3 -> state 0: duration alert");
        return 1;
    }
    else if (doorSensor.isDoorOpen())
    {
        stateHandler = state0_idle;
        Serial.println("state 3 -> state 0: door opened, session over");
        return 1;
    }
    return state3_stillness_timer;
}