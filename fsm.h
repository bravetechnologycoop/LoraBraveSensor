#ifndef FSM_H
#define FSM_H

#include "sensors.h"

namespace fsm
{
    /**
     * setup the FSM using values from EEPROM
     */
    void setupFSM();

    /**
     * timer getters
     * @returns the current timer value in ms
     */
    unsigned int getCountdownTimer();
    unsigned int getDurationTimer();
    unsigned int getStillnessTimer();

    /**
     * sets the countdown timer value stored in flash memory
     * @param countdownTimer the countdown timer value to store in ms
     * @returns countdown timer if successful, -1 otherwise
     */
    int setCountdownTimer(unsigned int timer);

    /**
     * sets the duration timer value stored in flash memory
     * @param durationTimer the duration timer value to store in ms
     * @returns duration timer if successful, -1 otherwise
     */
    int setDurationTimer(unsigned int timer);

    /**
     * sets the stillness timer value stored in flash memory
     * @param stillnessTimer the stillness timer value to store in ms
     * @returns stillness timer if successful, -1 otherwise
     */
    int setStillnessTimer(unsigned int timer);

    /**
     * resets the timers to their default values
     */
    void resetTimers(); 

    /**
     * states in the FSM
     * @return the sleep time in milliseconds
     */
    int state0_idle(DoorSensor doorSensor, MotionSensor motionSensor);
    int state1_countdown(DoorSensor doorSensor, MotionSensor motionSensor);
    int state2_duration(DoorSensor doorSensor, MotionSensor motionSensor);
    int state3_stillness(DoorSensor doorSensor, MotionSensor motionSensor);

    /**
     * the current state of the FSM
     * @returns the amount of time for the system to sleep in ms, until the next countdown threshold
     */
    typedef int (*stateHandler_t)(DoorSensor doorSensor, MotionSensor motionSensor);
    extern stateHandler_t stateHandler;
}

#endif // FSM_H