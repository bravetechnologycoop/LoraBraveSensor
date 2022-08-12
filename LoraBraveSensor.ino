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

DoorSensor doorSensor = DoorSensor(DOOR_SENSOR_PIN);
MotionSensor motionSensor = MotionSensor(MOTION_SENSOR_PIN);
AnalogSensor battery = AnalogSensor(BATTERY_PIN);

void setup()
{
  lora::setupOTAA();
  fsm::setupFSM();
  attachInterrupt(
      digitalPinToInterrupt(DOOR_SENSOR_PIN), [] {}, CHANGE);
  attachInterrupt(
      digitalPinToInterrupt(MOTION_SENSOR_PIN), [] {}, CHANGE);
  lora::sendUplink("Connected :3"); // No idea why first message always fails to send
}

void loop()
{
  int stateSleepTimer = fsm::stateHandler(doorSensor, motionSensor);
  int heartBeatTimer = heartbeat::getRemainingDuration();
  int sleepDuration; 
  if (stateSleepTimer == 0) {
    sleepDuration = heartBeatTimer;
  } else if (heartBeatTimer == 0) {
    sleepDuration = stateSleepTimer;
  } else {
    sleepDuration = min(stateSleepTimer, heartBeatTimer);
  }
  DEBUG_SERIAL_LOG.print("Sleeping for ");
  DEBUG_SERIAL_LOG.println(sleepDuration);
  if (sleepDuration > 0)
  {
    api.system.timer.create((RAK_TIMER_ID)FSM_TIMER, (RAK_TIMER_HANDLER)[](void *){}, RAK_TIMER_ONESHOT);
    api.system.timer.start((RAK_TIMER_ID)FSM_TIMER, sleepDuration, (void *)1);
  }
  api.system.sleep.all();
  DEBUG_SERIAL_LOG_MORE.println("Woke up");
}