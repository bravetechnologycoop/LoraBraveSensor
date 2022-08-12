#include <Arduino.h>
#include <ArduinoJson.h>
#include "fsm.h"
#include "sensors.h"
#include "lora.h"
#include "flashAddresses.h"
#include "main.h"

fsm::stateHandler_t fsm::stateHandler = fsm::state0_idle;
unsigned int lastStateHandleTime = millis();

static unsigned int COUNTDOWN_TIMER = 15000;
static unsigned int DURATION_TIMER = 30000;
static unsigned int STILLNESS_TIMER = 15000;

int state0_idle_timer = 0;
int state1_15sCountdown_timer = 0;
int state2_duration_timer = 0;
int state3_stillness_timer = 0;

void fsm::setupFSM()
{
    getFlash();
    DEBUG_SERIAL_LOG.printf("Timers constants: countdown: %u, duration: %u, stillness: %u\r\n", COUNTDOWN_TIMER, DURATION_TIMER, STILLNESS_TIMER);
}

bool fsm::getFlash()
{
    return api.system.flash.get(0, (uint8_t *)&COUNTDOWN_TIMER, sizeof(COUNTDOWN_TIMER)) &&
           api.system.flash.get(4, (uint8_t *)&DURATION_TIMER, sizeof(DURATION_TIMER)) &&
           api.system.flash.get(8, (uint8_t *)&STILLNESS_TIMER, sizeof(STILLNESS_TIMER));
}

bool fsm::setCountdownTimer(unsigned int timer)
{
    bool success = api.system.flash.set(COUNTDOWN_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(COUNTDOWN_TIMER));
    COUNTDOWN_TIMER = timer;
    return success;
}

bool fsm::setDurationTimer(unsigned int timer)
{
    bool success = api.system.flash.set(DURATION_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(DURATION_TIMER));
    DURATION_TIMER = timer;
    return success;
}

bool fsm::setStillnessTimer(unsigned int timer)
{
    bool success = api.system.flash.set(STILLNESS_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(STILLNESS_TIMER));
    STILLNESS_TIMER = timer;
    return success;
}

unsigned int fsm::getCountdownTimer()
{
    return COUNTDOWN_TIMER;
}

unsigned int fsm::getDurationTimer()
{
    return DURATION_TIMER;
}

unsigned int fsm::getStillnessTimer()
{
    return STILLNESS_TIMER;
}

int fsm::state0_idle(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state0_idle_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (motionSensor.isThereMotion() && !doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state1_15sCountdown;
        DEBUG_SERIAL_LOG.println("state 0 -> state 1: motion detected");

        state1_15sCountdown_timer = COUNTDOWN_TIMER;
        return state1_15sCountdown_timer;
    }
    return 0;
}

int fsm::state1_15sCountdown(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state1_15sCountdown_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (!motionSensor.isThereMotion() || doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("state 1 -> state 0: no motion, door closed");
        return 1;
    }
    else if (state1_15sCountdown_timer <= 0)
    {
        fsm::stateHandler = fsm::state2_duration;
        DEBUG_SERIAL_LOG.printf("state 1 -> state 2: motion detected for %u seconds\r\n", COUNTDOWN_TIMER / 1000);

        state2_duration_timer = DURATION_TIMER;
        return state2_duration_timer;
    }
    return state1_15sCountdown_timer;
}

int fsm::state2_duration(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state2_duration_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (!motionSensor.isThereMotion())
    {
        fsm::stateHandler = fsm::state3_stillness;
        DEBUG_SERIAL_LOG.println("state 2 -> state 3: no motion");

        state3_stillness_timer = STILLNESS_TIMER;
        return state3_stillness_timer;
    }
    else if (state2_duration_timer <= 0)
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("Duration Alert!!");
        DynamicJsonDocument doc(1024); 
        doc["alertType"] = "DurationAlert"; 
        lora::sendUplink(doc);
        DEBUG_SERIAL_LOG.println("state 2 -> state 0: duration alert");
        return 1;
    }
    else if (doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("state 2 -> state 0: door opened, session over");
        return 1;
    }
    return state2_duration_timer;
}

int fsm::state3_stillness(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state2_duration_timer -= millis() - lastStateHandleTime;
    state3_stillness_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (motionSensor.isThereMotion())
    {
        fsm::stateHandler = fsm::state2_duration;
        DEBUG_SERIAL_LOG.println("state 3 -> state 2: motion detected");
        return state2_duration_timer;
    }
    else if (state3_stillness_timer <= 0)
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("Stillness Alert!!");
        DynamicJsonDocument doc(1024); 
        doc["alertType"] = "StillnessAlert"; 
        lora::sendUplink(doc);
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: stillness alert");
        return 1;
    }
    else if (state2_duration_timer <= 0)
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("Duration Alert!!");
        DynamicJsonDocument doc(1024); 
        doc["alertType"] = "DurationAlert"; 
        lora::sendUplink(doc);
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: duration alert");
        return 1;
    }
    else if (doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: door opened, session over");
        return 1;
    }
    return state3_stillness_timer;
}