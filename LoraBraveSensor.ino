#include <Arduino.h>
#include "lora.h"
#include "sensors.h"
#include "fsm.h"
#include "flashAddresses.h"
#include "heartbeat.h"
#include "systemTimers.h"
#include "main.h"

#define DOOR_SENSOR_PIN PA7
#define MOTION_SENSOR_PIN PA6
#define BATTERY_PIN PA5
#define EEPROM_RESET_PIN PA4

DoorSensor doorSensor = DoorSensor(DOOR_SENSOR_PIN);
MotionSensor motionSensor = MotionSensor(MOTION_SENSOR_PIN);
AnalogSensor battery = AnalogSensor(BATTERY_PIN);

void setup()
{
  lora::setupOTAA();
  fsm::setupFSM();    
  heartbeat::setupHeartbeat(); 

  pinMode(EEPROM_RESET_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(EEPROM_RESET_PIN), resetEeprom, RISING);

  attachInterrupt(
      digitalPinToInterrupt(DOOR_SENSOR_PIN), [] {}, CHANGE);
  attachInterrupt(
      digitalPinToInterrupt(MOTION_SENSOR_PIN), [] {}, CHANGE);

  lora::sendUplink("Modify the comment if this gets sent :3"); // No idea why first message always fails to send
}

void loop()
{
  int stateSleepTimer = fsm::stateHandler(doorSensor, motionSensor);
  int heartBeatTimer = heartbeat::getRemainingDuration();
  DEBUG_SERIAL_LOG_MORE.printf("State sleep timer: %is\r\n", stateSleepTimer / 1000);
  DEBUG_SERIAL_LOG_MORE.printf("Heartbeat timer: %is\r\n", heartBeatTimer / 1000);
  int sleepDuration; 
  // Logic nessessary as timer == 0 equates infinite sleep, which is greater than any positive timer value
  if (stateSleepTimer == 0) {
    sleepDuration = heartBeatTimer;
  } else if (heartBeatTimer == 0) {
    sleepDuration = stateSleepTimer;
  } else {
    sleepDuration = min(stateSleepTimer, heartBeatTimer);
  }
  DEBUG_SERIAL_LOG.printf("Sleeping for %is\r\n", sleepDuration / 1000);
  if (sleepDuration > 0)
  {
    api.system.timer.create((RAK_TIMER_ID)FSM_TIMER, (RAK_TIMER_HANDLER)[](void *){}, RAK_TIMER_ONESHOT);
    api.system.timer.start((RAK_TIMER_ID)FSM_TIMER, sleepDuration, (void *)1);
  }
  api.system.sleep.all();
  DEBUG_SERIAL_LOG_MORE.println("Woke up");
}

void resetEeprom()
{
    DEBUG_SERIAL_LOG.printf("Resetting EEPROM\r\n");
    fsm::resetTimers(); 
    heartbeat::resetTimers(); 
}