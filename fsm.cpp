#include <Arduino.h>
#include <ArduinoJson.h>
#include "fsm.h"
#include "sensors.h"
#include "lora.h"
#include "flashAddresses.h"
#include "main.h"

fsm::stateHandler_t fsm::stateHandler = fsm::state0Idle;
unsigned int lastStateHandleTime = millis();

const unsigned int MAX_COUNTDOWN_TIMER = 20000;     // 20s
const unsigned int MIN_DURATION_TIMER = 0; 
const unsigned int MIN_STILLNESS_TIMER = 0; 
const unsigned int DEFAULT_COUNTDOWN_TIMER = 15000; // 15s
const unsigned int DEFAULT_DURATION_TIMER = 30000;  // 30s
const unsigned int DEFAULT_STILLNESS_TIMER = 15000; // 15s

static unsigned int countdownTimer = DEFAULT_COUNTDOWN_TIMER;
static unsigned int durationTimer = DEFAULT_DURATION_TIMER;
static unsigned int stillnessTimer = DEFAULT_STILLNESS_TIMER;

int state0_idle_timer = 0;
int state1_countdown_timer = 0;
int state2_duration_timer = 0;
int state3_stillness_timer = 0;

void fsm::setupFSM()
{
    bool success = api.system.flash.get(COUNTDOWN_TIMER_FLASH_ADDRESS, (uint8_t *)&countdownTimer, sizeof(countdownTimer)) &&
                   api.system.flash.get(DURATION_TIMER_FLASH_ADDRESS, (uint8_t *)&countdownTimer, sizeof(countdownTimer)) &&
                   api.system.flash.get(STILLNESS_TIMER_FLASH_ADDRESS, (uint8_t *)&stillnessTimer, sizeof(stillnessTimer));
    if (!success)
    {
        DEBUG_SERIAL_LOG.println("Error reading flash");
    }
    DEBUG_SERIAL_LOG.printf("Timers constants: countdown: %u, duration: %u, stillness: %u\r\n", countdownTimer, countdownTimer, stillnessTimer);
}

int fsm::setCountdownTimer(unsigned int timer)
{
    if (timer <= MAX_COUNTDOWN_TIMER)
    {
        bool success = api.system.flash.set(COUNTDOWN_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(countdownTimer));
        if (success)
        {
            countdownTimer = timer;
            DEBUG_SERIAL_LOG.printf("Countdown timer set to %u\r\n", countdownTimer);
            return countdownTimer;
        }
    }
    DEBUG_SERIAL_LOG.printf("Failed to set countdown timer to %u\r\n", timer);
    return -1;
}

int fsm::setDurationTimer(unsigned int timer)
{
    if (timer >= MIN_DURATION_TIMER)
    {
        bool success = api.system.flash.set(DURATION_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(countdownTimer));
        if (success)
        {
            countdownTimer = timer;
            DEBUG_SERIAL_LOG.printf("Duration timer set to %u\r\n", countdownTimer);
            return countdownTimer;
        }
    }
    DEBUG_SERIAL_LOG.printf("Failed to set duration timer to %u\r\n", timer);
    return -1;
}

int fsm::setStillnessTimer(unsigned int timer)
{
    if (timer >= MIN_STILLNESS_TIMER)
    {
        bool success = api.system.flash.set(STILLNESS_TIMER_FLASH_ADDRESS, (uint8_t *)&timer, sizeof(stillnessTimer));
        if (success)
        {
            stillnessTimer = timer;
            DEBUG_SERIAL_LOG.printf("Stillness timer set to %u\r\n", stillnessTimer);
            return stillnessTimer;
        }
    }
    DEBUG_SERIAL_LOG.printf("Failed to set stillness timer to %u\r\n", timer);
    return -1;
}

unsigned int fsm::getCountdownTimer()
{
    return countdownTimer;
}

unsigned int fsm::getDurationTimer()
{
    return countdownTimer;
}

unsigned int fsm::getStillnessTimer()
{
    return stillnessTimer;
}

int fsm::state0Idle(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state0_idle_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (motionSensor.isThereMotion() && !doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state1Countdown;
        DEBUG_SERIAL_LOG.println("state 0 -> state 1: motion detected");

        state1_countdown_timer = countdownTimer;
        return state1_countdown_timer;
    }
    return 0;
}

int fsm::state1Countdown(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state1_countdown_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (!motionSensor.isThereMotion() || doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0Idle;
        DEBUG_SERIAL_LOG.println("state 1 -> state 0: no motion, door closed");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    else if (state1_countdown_timer <= 0)
    {
        fsm::stateHandler = fsm::state2Duration;
        DEBUG_SERIAL_LOG.printf("state 1 -> state 2: motion detected for %u seconds\r\n", countdownTimer / 1000);

        state2_duration_timer = countdownTimer;
        return state2_duration_timer;
    }
    return state1_countdown_timer;
}

int fsm::state2Duration(DoorSensor doorSensor, MotionSensor motionSensor)
{
    state2_duration_timer -= millis() - lastStateHandleTime;
    lastStateHandleTime = millis();

    if (!motionSensor.isThereMotion())
    {
        fsm::stateHandler = fsm::state3_stillness;
        DEBUG_SERIAL_LOG.println("state 2 -> state 3: no motion");

        state3_stillness_timer = stillnessTimer;
        return min(state3_stillness_timer, state2_duration_timer); // For duration alert happening in state3_stillness
    }
    else if (state2_duration_timer <= 0)
    {
        fsm::stateHandler = fsm::state0Idle;
        DEBUG_SERIAL_LOG.println("Duration Alert!!");
        lora::uplinkMessage msg = {.alertType = lora::uplinkMessage::DURATION};
        lora::sendUplink(msg);
        DEBUG_SERIAL_LOG.println("state 2 -> state 0: duration alert");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    else if (doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0Idle;
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
        fsm::stateHandler = fsm::state2Duration;
        DEBUG_SERIAL_LOG.println("state 3 -> state 2: motion detected");
        return state2_duration_timer;
    }
    else if (state3_stillness_timer <= 0)
    {
        fsm::stateHandler = fsm::state0Idle;
        DEBUG_SERIAL_LOG.println("Stillness Alert!!");
        lora::uplinkMessage msg = {.alertType = lora::uplinkMessage::STILLNESS};
        lora::sendUplink(msg);
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: stillness alert");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    else if (state2_duration_timer <= 0)
    {
        fsm::stateHandler = fsm::state0Idle;
        DEBUG_SERIAL_LOG.println("Duration Alert!!");
        lora::uplinkMessage msg = {.alertType = lora::uplinkMessage::DURATION};
        lora::sendUplink(msg);
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: duration alert");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    else if (doorSensor.isDoorOpen())
    {
        fsm::stateHandler = fsm::state0Idle;
        DEBUG_SERIAL_LOG.println("state 3 -> state 0: door opened, session over");
        return 1; // In case after returning to state0, conditions are met to go to state1
    }
    return min(state3_stillness_timer, state2_duration_timer); // For duration alert happening in state3_stillness
}

void fsm::resetTimers()
{
    fsm::setCountdownTimer(DEFAULT_COUNTDOWN_TIMER);
    fsm::setDurationTimer(DEFAULT_DURATION_TIMER);
    fsm::setStillnessTimer(DEFAULT_STILLNESS_TIMER);
}