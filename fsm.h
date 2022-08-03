#ifndef FSM_H
#define FSM_H

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
 * @returns true if successful, false otherwise
 */
bool setCountdownTimer(unsigned int timer);

/**
 * sets the duration timer value stored in flash memory
 * @param durationTimer the duration timer value to store in ms
 * @returns true if successful, false otherwise
 */
bool setDurationTimer(unsigned int timer);

/**
 * sets the stillness timer value stored in flash memory
 * @param stillnessTimer the stillness timer value to store in ms
 * @returns true if successful, false otherwise
 */
bool setStillnessTimer(unsigned int timer);

/**
 * updates timer constants with the values stored in flash
 * @return true if successful, false otherwise
 */
bool getFlash();

/**
 * states in the FSM
 * @return the sleep time in milliseconds
 */
int state0_idle();
int state1_15sCountdown();
int state2_duration();
int state3_stillness();

/**
 * the current state of the FSM
 */
typedef int (*stateHandler_t)();
extern stateHandler_t stateHandler;

#endif // FSM_H