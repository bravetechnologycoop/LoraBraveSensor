#ifndef SYSTEM_TIMERS_H
#define SYSTEM_TIMERS_H

#include <Arduino.h>

const unsigned int FSM_TIMER = RAK_TIMER_0; // Timer to tell how long system to sleep for during each state
const unsigned int UPLINK_PROCESS_TIMER = RAK_TIMER_1; // Timer to change the uplink process to false after a certain amount of time

#endif // SYSTEM_TIMERS_H