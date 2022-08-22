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
int state1_countdown_timer = 0;
int state2_duration_timer = 0;
int state3_stillness_timer = 0;

void fsm::setupFSM()
{
    updateTimersFromFlash();
    DEBUG_SERIAL_LOG.printf("Timers constants: countdown: %u, duration: %u, stillness: %u\r\n", COUNTDOWN_TIMER, DURATION_TIMER, STILLNESS_TIMER);
}

bool fsm::updateTimersFromFlash()
{
    return api.system.flash.get(COUNTDOWN_TIMER_FLASH_ADDRESS, (uint8_t *)&COUNTDOWN_TIMER, sizeof(COUNTDOWN_TIMER)) &&
           api.system.flash.get(DURATION_TIMER_FLASH_ADDRESS, (uint8_t *)&DURATION_TIMER, sizeof(DURATION_TIMER)) &&
           api.system.flash.get(STILLNESS_TIMER_FLASH_ADDRESS, (uint8_t *)&STILLNESS_TIMER, sizeof(STILLNESS_TIMER)); 
}

int fsm::setCountdownTimer(unsigned int timer)
{
    bool success = api.system.flash.set(COUNTDOWN_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(COUNTDOWN_TIMER));
    COUNTDOWN_TIMER = timer;
    if (success)
    {
        DEBUG_SERIAL_LOG.printf("Countdown timer set to %u\r\n", COUNTDOWN_TIMER);
        return COUNTDOWN_TIMER; 
    }
    return -1;
}

int fsm::setDurationTimer(unsigned int timer)
{
    bool success = api.system.flash.set(DURATION_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(DURATION_TIMER));
    DURATION_TIMER = timer;
    if (success)
    {
        DEBUG_SERIAL_LOG.printf("Duration timer set to %u\r\n", DURATION_TIMER);
        return DURATION_TIMER;
    }
    return -1;
}

int fsm::setStillnessTimer(unsigned int timer)
{
    bool success = api.system.flash.set(STILLNESS_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(STILLNESS_TIMER));
    STILLNESS_TIMER = timer;
    if (success)
    {
        DEBUG_SERIAL_LOG.printf("Stillness timer set to %u\r\n", STILLNESS_TIMER);
        return STILLNESS_TIMER;
    }
    return -1;
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
        fsm::stateHandler = fsm::state1_countdown;
        DEBUG_SERIAL_LOG.println("state 0 -> state 1: motion detected");

        state1_countdown_timer = COUNTDOWN_TIMER;
        return state1_countdown_timer;
    }
    return 0;
}

int fsm::state1_countdown(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state1_countdown_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (!motionSensor.isThereMotion() || doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("state 1 -> state 0: no motion, door closed");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    else if (state1_countdown_timer <= 0)
    {
        fsm::stateHandler = fsm::state2_duration;
        DEBUG_SERIAL_LOG.printf("state 1 -> state 2: motion detected for %u seconds\r\n", COUNTDOWN_TIMER / 1000);

        state2_duration_timer = DURATION_TIMER;
        return state2_duration_timer;
    }
    return state1_countdown_timer;
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
        return min(state3_stillness_timer, state2_duration_timer); // For duration alert happening in state3_stillness
    }
    else if (state2_duration_timer <= 0)
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("Duration Alert!!");
        lora::uplinkMessage msg = {.alertType = lora::uplinkMessage::DURATION}; 
        lora::sendUplink(msg);
        DEBUG_SERIAL_LOG.println("state 2 -> state 0: duration alert");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    else if (doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("state 2 -> state 0: door opened, session over");
        return 1; // In case after returning to state0, conditions are met to go to state1
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
        lora::uplinkMessage msg = {.alertType = lora::uplinkMessage::STILLNESS}; 
        lora::sendUplink(msg);
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: stillness alert");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    else if (state2_duration_timer <= 0)
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("Duration Alert!!");
        lora::uplinkMessage msg = {.alertType = lora::uplinkMessage::DURATION}; 
        lora::sendUplink(msg);
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: duration alert");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    else if (doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0_idle;
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: door opened, session over");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    return min(state3_stillness_timer, state2_duration_timer); // For duration alert happening in state3_stillness
}