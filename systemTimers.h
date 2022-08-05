#ifndef SYSTEM_TIMERS_H
#define SYSTEM_TIMERS_H

#include <Arduino.h>

const unsigned int FSM_TIMER = RAK_TIMER_0; // timer to tell how long system to sleep for during each state
const unsigned int HEARTBEAT_TIMER = RAK_TIMER_1; // timer to send heartbeat to server

#endif // SYSTEM_TIMERS_H