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
     * adds to the fsm queue the next dataset for the FSM to process
     */
    void addToSensorDataQueue();

     /**
     * check if the fsm queue is empty
     * @return true if empty, false otherwise
     */
    bool isSensorDataQueueEmpty();

    /**
     * runs instance of state machine
     * @return duration for device to sleep for
     */
    int handleState();

    /**
     * timer getters
     * @returns the current timer value in ms
     */
    unsigned int getCountdownTimer();
    unsigned int getDurationTimer();
    unsigned int getStillnessTimer();

    /**
     * sets the timer value stored in flash memory
     * @param timer the  timer value to store in ms
     * @returns timer value in ms if successful, -1 otherwise
     */
    int setCountdownTimer(unsigned int timer);
    int setDurationTimer(unsigned int timer);
    int setStillnessTimer(unsigned int timer);

    /**
     * resets the timers to their default values
     */
    void resetTimers();

    /**
     * states in the FSM
     * @return the sleep time in milliseconds
     */
    int state0Idle(SensorData sensorData);
    int state1Countdown(SensorData sensorData);
    int state2Duration(SensorData sensorData);
    int state3Stillness(SensorData sensorData);

    /**
     * the current state of the FSM
     * @returns the amount of time for the system to sleep in ms, until the next countdown threshold
     */
    typedef int (*stateHandler_t)(SensorData sensorData);
    extern stateHandler_t stateHandler;
}

#endif // FSM_H